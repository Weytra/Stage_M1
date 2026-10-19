#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "Structures.h"
#include "Fonctions.h"
#include "fonctions_mpi.h"
#include "gest_arg.h"
#include"energy_monitor.h"

#define MCW MPI_COMM_WORLD

static void local_analysis(grid_local *G, int counts[4]) {
    counts[0] = counts[1] = counts[2] = counts[3] = 0;

    int nx = G->nx;
    int ny = G->ny_local;

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

    //donne pour le benchmark
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

    //bench
    double algo_start_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[0] = read_energy_uj();

    for (int it = 0; it < nb_it; it++) {
        exchange_borders(G, MCW, top_border, bottom_border);
        next_step_local_mg(G, top_border, bottom_border,MCW);
        //next_step_local(G, top_border, bottom_border);

        if (my_rank == 0 && (nb_it <= 20 || (it % (nb_it / 10 + 1) == 0))) {
            printf("Iteration %d / %d\n", it, nb_it);
        }
    }

    MPI_Barrier(MCW);
    
    double algo_end_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[1] = read_energy_uj();

    
    int local_counts[4];
    int global_counts[4];

    //local_analysis(G, local_counts);
    MPI_Reduce(local_counts, global_counts, 4, MPI_INT, MPI_SUM, 0, MCW);

    if (my_rank == 0) {
        printf("\nSimulation terminee.\n");
        // printf("Temps max (sur tous les processus) : %f s\n", max_time);
        // printf("S finaux : %d\n", global_counts[0]);
        // printf("E finaux : %d\n", global_counts[1]);
        // printf("I finaux : %d\n", global_counts[2]);
        // printf("R finaux : %d\n", global_counts[3]);
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

        //FullBenchResult F_b_r = {R_global,R_algo};
        printf("GLOBAL_TIME=%lf\n",
            R_global.time_s);

        printf("GLOBAL_ENERGY=%.12lf\n",
            R_global.energy_j);

        printf("GLOBAL_POWER=%lf\n",
            R_global.avg_power_w);

        printf("ALGO_TIME=%lf\n",
            R_algo.time_s);

        printf("ALGO_ENERGY=%.12lf\n",
            R_algo.energy_j);

        printf("ALGO_POWER=%lf\n",
            R_algo.avg_power_w);
    }


    MPI_Finalize();
    return 0;
}
