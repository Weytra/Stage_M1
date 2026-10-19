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
#include "../../../include/headers/for_fox.h"



//  C(m,p) = A(m,n) * B(n,p)
int main(int argc, char ** argv){
    arg * ARG = parse_arg(argc,argv);
    int size = ARG->size;
    int block = ARG->block;
    //donne pour le benchmark
    long long global_energy[2];
    long long algo_energy[2];

    //double global_start_time = MPI_Wtime(); //a adapter pour du non mpi
    double global_start_time = now();
    global_energy[0] = read_energy_uj();


    int *A = malloc(size * size * sizeof(int));
    int *B = malloc(size * size * sizeof(int));
    int *C = calloc(size * size,sizeof(int));
    for(int i = 0;i<size;i++){
        for(int j = 0;j<size;j++){
            A[i*size +j] = i*size+j +1;
            B[i*size +j] = (i == j) ? 1:0;

        }
    }

    transpose(B,size);
    // struct timespec start, end;
    // clock_gettime(CLOCK_MONOTONIC, &start);
    double algo_start_time = now();
    algo_energy[0] = read_energy_uj();
      

    // int sum;
    if(block){
        for (int ii = 0; ii < size; ii += block) {
            for (int jj = 0; jj < size; jj += block) {
                for (int kk = 0; kk < size; kk += block) {

                    int i_end = (ii + block < size) ? ii + block : size;
                    int j_end = (jj + block < size) ? jj + block : size;
                    int k_end = (kk + block < size) ? kk + block : size;

                    for (int i = ii; i < i_end; i++) {
                        int idc = i * size;

                        for (int j = jj; j < j_end; j++) {

                            int sum = 0;

                            for (int k = kk; k < k_end; k++) {
                                sum += A[idc+k] * B[j*size+k];
                            }

                            C[idc+j] += sum;
                        }
                    }
                }
            }
        }
    }
    else{
        for(int i = 0; i < size; i++){
            int rowA = i * size;

            for(int j = 0; j < size; j++){

                int sum = 0;

                for(int k = 0; k < size; k++){
                    sum += A[rowA + k] * B[j*size + k];
                }

                C[rowA + j] = sum;
            }
        }
    }

    if(ARG->mode == 1 && argc == 5 || (ARG->mode ==2 && argc == 6)){
        char buff[128],buff_2[128];
        sprintf(buff,"%s.mat",ARG->file_C);
        ecriture(C,size,size,buff);
        sprintf(buff_2,"%s.txt",ARG->file_C);
        ecriture_txt(C,size,size,buff_2);
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