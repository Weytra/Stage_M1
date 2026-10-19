#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>

#include "../../../include/headers/affichage.h"
#include "../../../include/headers/lecture_mat.h"
#include "../../../include/headers/resize.h"
#include "../../../include/headers/gest_arg.h"
#include "../../../include/headers/energy_monitor.h"
#include "../../../include/headers/vecto.h"



//  C(m,p) = A(m,n) * B(n,p)
int main(int argc, char ** argv){
    arg * ARG = parse_arg(argc,argv);
    int size = ARG->size;
    int block = ARG->block;
    //donne pour le benchmark
    long long global_energy[2];
    long long algo_energy[2];

    double global_start_time = now();
    global_energy[0] = read_energy_uj();
      
    srand(time(NULL));

    int *A = malloc(size * size * sizeof(int));
    int *B = malloc(size * size * sizeof(int));
    int *C = malloc(size * size * sizeof(int));
    for(int i = 0;i<size*size;i++){
        A[i] = rand()%1000;
        B[i] = rand()%1000;
    }

    // struct timespec start, end;
    // clock_gettime(CLOCK_MONOTONIC, &start);
    double algo_start_time = now();
    algo_energy[0] = read_energy_uj();

    if (block) {
        matmul_block_avx512_transposed(A, B,C, size, block);
    } else {
        matmul_avx512_transposed(A, B, C, size);
    }


    double algo_end_time = now();
    algo_energy[1] = read_energy_uj();

    free(A);
    free(B);
    free(C);
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