#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#include "../../include/headers/fonctions.h"
#include "../../include/headers/gest_arg_init.h"

#include "../../include/src/fonctions.c"
#include "../../include/src/gest_arg_init.c"
#include "../../include/src/energy_monitor.c"


unsigned char * next_step_lin_omp(unsigned char * matrice,int size){
    unsigned char *res = alloc_matrice_lin(size,size);

    #pragma omp parallel for collapse(2)
    for(int i = 1;i< size -1;i++){
        for(int j = 1; j<size-1;j++){
            int cpt = 0;

            for(int k = -1; k<2;k++){
                for(int m = -1; m<2;m++){
                    cpt += matrice[(i+k)*size + j+m];
                }
            }

            cpt -= matrice[i*size + j];

            if(cpt == 3)
                res[i*size + j] = 1;
            else if(cpt == 2)
                res[i*size + j] = matrice[i*size + j];
            else
                res[i*size + j] = 0;
        }
    }

    free(matrice);
    return res;
}


int main(int argc, char ** argv){
    srand(100);
    argument * ARG = init_arg(argc,argv);
    int nb_it = ARG->nb_it;
    int size = ARG->taille;

    long long global_energy[2];
    long long algo_energy[2];
    double global_start_time = now();
    global_energy[0] = read_energy_uj();

    // première approche 
    // definition de la matrice en format classique
    unsigned char *matrice = alloc_matrice_lin(size,size);
    init_mat_lin(matrice,size,ARG->type,ARG->size);

    printf("on execute avec size %d nb it %d \n",size,nb_it);
    printf("____________________\n");

    //affichage_lin(matrice,size);
    double algo_start_time = now();
    algo_energy[0] = read_energy_uj();

    
    for(int i = 0;i<nb_it;i++){
        matrice = next_step_lin_omp(matrice,size);
        // printf("____________________\n");
        // affichage_lin(matrice,size);
    }

    double algo_end_time = now();

    algo_energy[1] = read_energy_uj();
    clock_t fin = clock();
    //affichage_lin(matrice,size);

    free(matrice);  
    free(ARG);

    double global_end_time = now();
    global_energy[1] = read_energy_uj();

    {// benchmark
    BenchResult R_global;
    benchmark_end(&R_global,global_energy[0],global_start_time,global_energy[1],global_end_time);

    BenchResult R_algo;
    benchmark_end(&R_algo,algo_energy[0],algo_start_time,algo_energy[1],algo_end_time);

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
    return 0;
}
