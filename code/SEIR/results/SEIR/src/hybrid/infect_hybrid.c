#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#include "Structures.h"
#include "Fonctions.h"
#include "fonctions_mpi.h"
#include "gest_arg.h"
#include"energy_monitor.h"

#define MCW MPI_COMM_WORLD

typedef struct {
    individus *ind;
    int ni;
    int nj;
} move_t;

static void local_analysis(grid_local *G, int counts[4]) {
    counts[0] = counts[1] = counts[2] = counts[3] = 0;

    int nx = G->nx;
    int ny = G->ny_local;

    #pragma omp parallel for collapse(2) reduction(+:counts[0],counts[1],counts[2],counts[3])
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            cellule *c = &G->act[i][j];
            for (int k = 0; k < c->nb_indiv; k++) {
                switch (c->indiv[k]->status) {
                    case S: counts[0]++; break;
                    case E: counts[1]++; break;
                    case I: counts[2]++; break;
                    case R: counts[3]++; break;
                    default: break;
                }
            }
        }
    }
}


void next_step_local_mg_omp(grid_local *G,
                            border_cell *top_border,
                            border_cell *bottom_border,
                            MPI_Comm comm,int cz)
{
    int nx = G->nx;
    int ny = G->ny_local;

    cellule **act  = G->act;
    cellule **next = G->next;

    int T = omp_get_max_threads();
    fprintf(stderr,"on a %d thread\n",T);

    // -----------------------------
    // buffers thread-local
    // -----------------------------
    typedef struct {
        move_t *data;
        int count;
        int cap;
    } move_buf;

    move_buf *bufs = calloc(T, sizeof(move_buf));

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        bufs[tid].cap = 1024;
        bufs[tid].count = 0;
        bufs[tid].data = malloc(bufs[tid].cap * sizeof(move_t));

        #pragma omp for collapse(2)
        for(int i = 0; i < ny; i++)
        {
            for(int j = 0; j < nx; j++)
            {
                cellule *c = &act[i][j];

                for(int k = 0; k < c->nb_indiv; k++)
                {
                    individus *ind = c->indiv[k];

                    int dx = rand_int(-1, 1);
                    int dy = rand_int(-1, 1);

                    int ni = i + dy;
                    int nj = (j + dx + nx) % nx;

                    move_buf *b = &bufs[tid];

                    if(b->count == b->cap)
                    {
                        b->cap *= 2;
                        b->data = realloc(b->data, b->cap * sizeof(move_t));
                    }

                    b->data[b->count].ind = ind;
                    b->data[b->count].ni  = ni;
                    b->data[b->count].nj  = nj;
                    b->count++;
                }
            }
        }
    }

    // -----------------------------
    // reset next
    // -----------------------------
    for(int i = 0; i < ny; i++)
    {
        for(int j = 0; j < nx; j++)
        {
            free(next[i][j].indiv);
            next[i][j].indiv = NULL;
            next[i][j].nb_indiv = 0;
        }
    }

    // -----------------------------
    // buffers MPI migration
    // -----------------------------
    indiv_buffer send_up, send_down;
    buffer_init(&send_up);
    buffer_init(&send_down);

    // -----------------------------
    // fusion thread-local
    // -----------------------------
    for(int t = 0; t < T; t++)
    {
        for(int k = 0; k < bufs[t].count; k++)
        {
            move_t m = bufs[t].data[k];

            if(m.ni < 0)
            {
                buffer_push(&send_up, m.ind);
                continue;
            }

            if(m.ni >= ny)
            {
                buffer_push(&send_down, m.ind);
                continue;
            }

            cellule *c = &next[m.ni][m.nj];

            c->indiv = realloc(c->indiv,
                               (c->nb_indiv + 1) * sizeof(individus*));

            c->indiv[c->nb_indiv++] = m.ind;
        }
    }

    // free buffers OpenMP
    for(int t = 0; t < T; t++)
        free(bufs[t].data);

    free(bufs);

    // -----------------------------
    // MPI migration (inchangé)
    // -----------------------------
    indiv_buffer recv_up, recv_down;
    buffer_init(&recv_up);
    buffer_init(&recv_down);

    int my_rank, comm_size;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &comm_size);

    int up   = (my_rank == 0) ? comm_size - 1 : my_rank - 1;
    int down = (my_rank == comm_size - 1) ? 0 : my_rank + 1;

    if(up != MPI_PROC_NULL)
        send_buffer(&send_up, up, 100, comm);

    if(down != MPI_PROC_NULL)
        send_buffer(&send_down, down, 200, comm);

    if(up != MPI_PROC_NULL)
        recv_buffer(&recv_up, up, 200, comm);

    if(down != MPI_PROC_NULL)
        recv_buffer(&recv_down, down, 100, comm);

    if(up != MPI_PROC_NULL)
        insert_received_individuals(G, &recv_up, 0);

    if(down != MPI_PROC_NULL)
        insert_received_individuals(G, &recv_down, ny - 1);

    // -----------------------------
    // infection + évolution (OMP SAFE)
    // -----------------------------
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < ny; i++)
    {
        for(int j = 0; j < nx; j++)
        {
            cellule *c = &next[i][j];
            if(c->nb_indiv == 0) continue;

            int infect = 0;

            for(int k = 0; k < c->nb_indiv; k++)
                if(c->indiv[k]->status == I)
                    infect++;

            for(int p = -1; p <= 1; p++)
            for(int q = -1; q <= 1; q++)
            {
                int ni = i + p;
                int nj = (j + q + nx) % nx;

                if(ni < 0)
                    infect += top_border[nj].nb_I;
                else if(ni >= ny)
                    infect += bottom_border[nj].nb_I;
                else
                    for(int k = 0; k < next[ni][nj].nb_indiv; k++)
                        if(next[ni][nj].indiv[k]->status == I)
                            infect++;
            }

            for(int k = 0; k < c->nb_indiv; k++)
            {
                individus *ind = c->indiv[k];

                if(ind->status == S && infection(infect))
                {
                    ind->status = E;
                    ind->tps_in_status = 0;
                }

                ind->tps_in_status++;

                if(ind->status == E && ind->tps_in_status >= ind->dE)
                    ind->status = I, ind->tps_in_status = 0;

                if(ind->status == I && ind->tps_in_status >= ind->dI)
                    ind->status = R, ind->tps_in_status = 0;

                if(ind->status == R && ind->tps_in_status >= ind->dR)
                    ind->status = S, ind->tps_in_status = 0;
            }
        }
    }

    // -----------------------------
    // swap
    // -----------------------------
    cellule **tmp = G->act;
    G->act = G->next;
    G->next = tmp;

    G->temps++;
}


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int my_rank, comm_size;
    MPI_Comm_rank(MCW, &my_rank);
    MPI_Comm_size(MCW, &comm_size);

    arg * ARG = parse_arg(argc,argv);

    int nb_it        = ARG->nb_it;
    int size_grille  = ARG->size_grille;
    int nb_individus = ARG->nb_individuis;
    int nb_infect    = ARG->nb_infect;

    fprintf(stderr,"on a une grille de %d avec %d indiv et %d infect, sur %d it\n",
        size_grille,nb_individus,nb_infect,nb_it);

    long long global_energy[2];
    long long algo_energy[2];
    MPI_Barrier(MCW);
    double global_start_time = MPI_Wtime();
    if(!my_rank)
        global_energy[0] = read_energy_uj();

    int params[4];
    if (my_rank == 0) {
        params[0] = nb_it;
        params[1] = size_grille;
        params[2] = nb_individus;
        params[3] = nb_infect;
    }
    MPI_Bcast(params, 4, MPI_INT, 0, MCW);

    nb_it        = params[0];
    size_grille  = params[1];
    nb_individus = params[2];
    nb_infect    = params[3];

    int nx = size_grille;
    int ny_global = size_grille;

    int base = ny_global / comm_size;
    int reste = ny_global % comm_size;

    int ny_local;
    int y_offset;

    if (my_rank < reste) {
        ny_local = base + 1;
        y_offset = my_rank * (base + 1);
    } else {
        ny_local = base;
        y_offset = reste * (base + 1) + (my_rank - reste) * base;
    }

    double frac_local = (double)ny_local / (double)ny_global;
    int nb_indiv_local  = (int)(nb_individus * frac_local);
    int nb_infect_local = (int)(nb_infect    * frac_local);

    srand(100 + my_rank);

    grid_local *G = make_local_grid(nx, ny_local, y_offset);
    init_local_population(G, nb_indiv_local, nb_infect_local);

    border_cell *top_border    = malloc(nx * sizeof(border_cell));
    border_cell *bottom_border = malloc(nx * sizeof(border_cell));

    MPI_Barrier(MCW);

    double algo_start_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[0] = read_energy_uj();

    for (int it = 0; it < nb_it; it++) {
        exchange_borders(G, MCW, top_border, bottom_border);
        next_step_local_mg_omp(G, top_border, bottom_border,MCW,comm_size);
    }

    MPI_Barrier(MCW);

    double algo_end_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[1] = read_energy_uj();


    int local_counts[4];
    int global_counts[4];

    local_analysis(G, local_counts);
    MPI_Reduce(local_counts, global_counts, 4, MPI_INT, MPI_SUM, 0, MCW);

    if (my_rank == 0) {
        printf("\nSimulation terminee.\n");
        printf("S finaux : %d\n", global_counts[0]);
        printf("E finaux : %d\n", global_counts[1]);
        printf("I finaux : %d\n", global_counts[2]);
        printf("R finaux : %d\n", global_counts[3]);
    }

    free(top_border);
    free(bottom_border);
    free_local_grid(G);

    MPI_Barrier(MCW);
    double global_end_time = MPI_Wtime();
    if(!my_rank)
        global_energy[1] = read_energy_uj();

    double max_global_end_time;
    double min_global_start_time;

    double max_algo_end_time;
    double min_algo_start_time;

    MPI_Reduce(&global_end_time,&max_global_end_time,1,MPI_DOUBLE,MPI_MAX,0,MCW);
    MPI_Reduce(&global_start_time,&min_global_start_time,1,MPI_DOUBLE,MPI_MIN,0,MCW);

    MPI_Reduce(&algo_end_time,&max_algo_end_time,1,MPI_DOUBLE,MPI_MAX,0,MCW);
    MPI_Reduce(&algo_start_time,&min_algo_start_time,1,MPI_DOUBLE,MPI_MIN,0,MCW);

    if(my_rank == 0)
    {
        BenchResult R_global;
        benchmark_end(&R_global,global_energy[0],min_global_start_time,global_energy[1],max_global_end_time);

        BenchResult R_algo;
        benchmark_end(&R_algo,algo_energy[0],min_algo_start_time,algo_energy[1],max_algo_end_time);

        printf("GLOBAL_TIME=%lf\n", R_global.time_s);
        printf("GLOBAL_ENERGY=%.12lf\n", R_global.energy_j);
        printf("GLOBAL_POWER=%lf\n", R_global.avg_power_w);
        printf("ALGO_TIME=%lf\n", R_algo.time_s);
        printf("ALGO_ENERGY=%.12lf\n", R_algo.energy_j);
        printf("ALGO_POWER=%lf\n", R_algo.avg_power_w);
    }

    MPI_Finalize();
    return 0;
}
