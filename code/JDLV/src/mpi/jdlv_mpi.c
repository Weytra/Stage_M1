#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#include "../../include/headers/fonctions.h"
#include "../../include/headers/gest_arg_init.h"
#include "../../include/headers/energy_monitor.h"


#include "../../include/src/fonctions.c"
#include "../../include/src/gest_arg_init.c"
#include "../../include/src/energy_monitor.c"

#define MCW MPI_COMM_WORLD

// Échange des halos haut/bas
void exchange_halos(unsigned char *local, int size, int local_h, int my_rank, int comm_size) {
    int up   = (my_rank == 0) ? MPI_PROC_NULL : my_rank - 1;
    int down = (my_rank == comm_size-1) ? MPI_PROC_NULL : my_rank + 1;

    // envoyer ligne 1 vers up, recevoir halo bas (ligne local_h+1) depuis down
    MPI_Sendrecv(local + 1*size,        size, MPI_UNSIGNED_CHAR, up,   0,
                 local + (local_h+1)*size, size, MPI_UNSIGNED_CHAR, down, 0,
                 MCW, MPI_STATUS_IGNORE);

    // envoyer ligne local_h vers down, recevoir halo haut (ligne 0) depuis up
    MPI_Sendrecv(local + local_h*size,  size, MPI_UNSIGNED_CHAR, down, 1,
                 local + 0*size,        size, MPI_UNSIGNED_CHAR, up,   1,
                 MCW, MPI_STATUS_IGNORE);
}



int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int my_rank, comm_size;
    MPI_Comm_rank(MCW, &my_rank);
    MPI_Comm_size(MCW, &comm_size);


    srand(100);
    argument * ARG = init_arg(argc,argv);
    int nb_it = ARG->nb_it;
    int size = ARG->taille;

     //donne pour le benchmark
    long long global_energy[2];
    long long algo_energy[2];
    MPI_Barrier(MCW);
    double global_start_time = MPI_Wtime();
    if(!my_rank)
        global_energy[0] = read_energy_uj();



    printf(" size = %d,nb it = %d\n",size,nb_it);


    if (size % comm_size != 0) {
        if (my_rank == 0)
            fprintf(stderr, "Erreur: size doit être divisible par comm_size\n");
        MPI_Abort(MCW, EXIT_FAILURE);
    }

    int local_h = size / comm_size;

    unsigned char *global = NULL;
    if (my_rank == 0) {
        global = alloc_matrice_lin(size, size);
        init_mat_lin(global,size,ARG->type,ARG->size);
        printf("_______________\n");
        affichage_lin(global,size);
        printf("_______________\n");

    }

    // local avec halos : (local_h + 2) x size
    unsigned char *local      = alloc_matrice_lin(local_h + 2, size);
    unsigned char *local_next = alloc_matrice_lin(local_h + 2, size);

    // Scatter des lignes globales vers les parties locales (sans halos)
    unsigned char *local_core = local + 1*size; // ligne 1
    MPI_Scatter(global, local_h*size, MPI_UNSIGNED_CHAR,
                local_core, local_h*size, MPI_UNSIGNED_CHAR,
                0, MCW);




    MPI_Barrier(MCW);
    //bench
    double algo_start_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[0] = read_energy_uj();


    for (int it = 0; it < nb_it; it++) {
        // échange des halos
        exchange_halos(local, size, local_h, my_rank, comm_size);

        // calcul local
        next_step_local(local, local_next, size, local_h);

        // ping-pong des buffers
        unsigned char *tmp = local;
        local      = local_next;
        local_next = tmp;
    }

    MPI_Barrier(MCW);
    
    double algo_end_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[1] = read_energy_uj();


    // Gather final pour reconstruire la grille globale
    unsigned char *local_core_final = local + 1*size;
    MPI_Gather(local_core_final, local_h*size, MPI_UNSIGNED_CHAR,
               global,          local_h*size, MPI_UNSIGNED_CHAR,
               0, MCW);

    if (my_rank == 0) {
        printf("Résultat final après %d itérations:\n", nb_it);
        affichage_lin(global, size);
        free(global);
    }

    free(local);
    free(local_next);


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
