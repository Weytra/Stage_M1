#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>

#include "../include/headers/affichage.h"
#include "../include/headers/lecture_mat.h"
#include "../include/headers/resize.h"
#include "../include/headers/gest_arg.h"
#include "../include/headers/energy_monitor.h"
#include "../include/headers/vecto.h"


//  C(m,p) = A(m,n) * B(n,p)
int main(int argc, char ** argv){
    arg * ARG = parse_arg(argc,argv);
    int size = ARG->size;
    int block = ARG->block;
    //donne pour le benchmark
    long long global_energy[2];
    long long algo_energy[2];

    //double global_start_time = MPI_Wtime(); //a adapter pour du non mpi
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

    // int sum;
    if(block){
        for (int ii = 0; ii < size; ii += block) {
            for (int jj = 0; jj < size; jj += block) {
                for (int kk = 0; kk < size; kk += block) {

                    int i_end = (ii + block < size) ? ii + block : size;
                    
                    for (int i = ii; i < i_end; i++) {
                        int idc = i*size;
                        int j_end = (jj + block < size) ? jj + block : size;
                        for (int j = jj; j < j_end; j++) {
                            
                            int sum = 0; // cohérent avec ta version non-blockée
                            int k_end = (kk + block < size) ? kk + block : size;
                            __m512i vsum = _mm512_setzero_si512();

                            for (int k = kk; k + 16 <= k_end; k += 16) {

                                __m512i va = _mm512_loadu_si512((__m512i*)&A[i*size + k]);

                                __m512i idx = _mm512_set_epi32(
                                    (k+15)*size + j, (k+14)*size + j, (k+13)*size + j, (k+12)*size + j,
                                    (k+11)*size + j, (k+10)*size + j, (k+9)*size + j,  (k+8)*size + j,
                                    (k+7)*size + j,  (k+6)*size + j,  (k+5)*size + j,  (k+4)*size + j,
                                    (k+3)*size + j,  (k+2)*size + j,  (k+1)*size + j,  (k+0)*size + j
                                );

                                __m512i vb = _mm512_i32gather_epi32(idx, B, 4);

                                vsum = _mm512_add_epi32(vsum, _mm512_mullo_epi32(va, vb));
                            }

                            C[idc + j] = sum;
                        }
                    }

                }
            }
        }
    }
    else{
        for(int k = 0;k<block;k++){
                int rowA = k*block;
                for(int m = 0;m<block;m++){
                    int idc  = rowA+m;
                    int sum = 0;
                    for(int p = 0;p<block;p++){
                        sum += A[rowA+p]*B[p*block+m];
                    }
                    C[idc] = sum;
                }
            }
    }


//     clock_gettime(CLOCK_MONOTONIC, &end);

//     double elapsed = (end.tv_sec - start.tv_sec)
//                + (end.tv_nsec - start.tv_nsec) / 1e9;

// printf("Temps d'execution = %.6f secondes\n", elapsed);
    free(A);
    free(B);
    free(C);
    free(ARG);
    
    return 0;
}