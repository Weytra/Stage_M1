#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

#include "../include/headers/affichage.h"
#include "../include/headers/for_fox.h"
#include "../include/headers/lecture_mat.h"
#include "../include/headers/resize.h"
#include "../include/headers/gest_arg.h"
#include "../include/headers/energy_monitor.h"


#define MCW MPI_COMM_WORLD
#define MSI MPI_STATUS_IGNORE

int main(int argc, char ** argv){
    arg * ARG = parse_arg(argc,argv);

    int size = ARG->size;//taille des matrices

    int comm_size,my_rank;

    srand(time(NULL));

    MPI_Init(NULL,NULL);

    MPI_Comm_size(MCW,&comm_size);
    MPI_Comm_rank(MCW,&my_rank);

    //donne pour le benchmark
    long long global_energy[2];
    long long algo_energy[2];

    MPI_Barrier(MCW);
    double global_start_time = MPI_Wtime();
    if(!my_rank)
        global_energy[0] = read_energy_uj();

        
    int block;

    int *mat_A;
    int *mat_B;
    int *mat_C;
    int * A_short;
    // int * B_short;
    int * C_short;

    int size_b_p;//size by proc

    if (size % comm_size != 0) {
        if (my_rank == 0)
            fprintf(stderr, "Erreur: size (%d) doit être multiple de comm_size (%d)\n", size, comm_size);
        MPI_Abort(MCW, 1);
    }

    size_b_p = size/comm_size * size;
    block = size/comm_size;

    mat_B = malloc(size * size * sizeof(int));

    if(my_rank == 0){//allocation des donnes pour p0
        if(ARG->mode == 1){
            printf("on entre dans mode 1\n");
            mat_A = malloc(size*size*sizeof(int));
            mat_B = calloc(size*size,sizeof(int));
            
            //initialisation des donnees
            for(int i = 0;i<size;i++){
                for(int j = 0;j<size;j++){
                    mat_A[i*size+j] = i*size+j+1;
                }
                mat_B[i*size +i] = 1;//matrice identité
            }
        }
        else
        if(ARG->mode == 2){
            printf("on entre dans mode 2\n");

            Matrix M_A = lecture(ARG->file_A);
            Matrix M_B = lecture(ARG->file_B);
            mat_A = M_A.data;
            mat_B = M_B.data;

            int old_size = max(M_A.cols,M_A.rows,M_B.cols,M_B.rows);
            
            int to_resize = (comm_size - old_size%comm_size)%comm_size;//indique le nombre de ligne manquante (on reste sur des matrice carrées)
        
            size = old_size+to_resize;

            if(size != old_size || size != min(M_A.cols,M_A.rows,M_B.cols,M_B.rows)){
                mat_A = resize(mat_A,M_A.rows,M_A.cols,size,size);
                mat_B = resize(mat_B,M_B.rows,M_B.cols,size,size);
            }
        }
        else{
            fprintf(stdout,"on est pas cense etre la ARRET\n");
            exit(EXIT_FAILURE);
        }

        //aff_mat(A_blk,size);
        
        mat_C = calloc(size*size,sizeof(int));
    }

    A_short = malloc(size_b_p*sizeof(int));
    C_short = malloc(size_b_p*sizeof(int));
    memset(C_short, 0, size_b_p * sizeof(int));

    MPI_Barrier(MCW);

    double algo_start_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[0] = read_energy_uj();

    MPI_Scatter(mat_A, size_b_p, MPI_INT,A_short, size_b_p, MPI_INT,0, MCW);
    MPI_Bcast(mat_B, size*size,MPI_INT,0, MCW);
    
    //on considere que la matrice B est en realité la transposé de la matrice reelle
    //calculs
    int res;

    if(!ARG->block){
        for(int i = 0;i<size/comm_size;i++){
            for (int j = 0;j<size;j++){
                res = 0;
                for(int k = 0;k<size;k++){
                    res += A_short[i*size+k]*mat_B[j*size + k ];
                }
                C_short[i*size+j] = res;
            }
        }
    }else{
        int local_rows = size / comm_size;
        for (int i = 0; i < local_rows; i += block) {
            for (int j = 0; j < size; j += block) {
                for (int k = 0; k < size; k += block) {

                    int i_max = (i + block > local_rows) ? local_rows : i + block;
                    int j_max = (j + block > size) ? size : j + block;
                    int k_max = (k + block > size) ? size : k + block;

                    for (int ii = i; ii < i_max; ii++) {
                        for (int jj = j; jj < j_max; jj++) {

                            int res = C_short[ii * size + jj];

                            for (int kk = k; kk < k_max; kk++) {
                                res += A_short[ii * size + kk]
                                    * mat_B[jj * size + kk];
                            }

                            C_short[ii * size + jj] = res;
                        }
                    }
                }
            }
        }
    }
    MPI_Gather(C_short,size_b_p,MPI_INT,mat_C,size_b_p,MPI_INT,0,MCW);
    MPI_Barrier(MCW);
    
    double algo_end_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[1] = read_energy_uj();
    if (my_rank == 0) {
        if(ARG->mode == 1 && argc == 5 || (ARG->mode ==2 && argc == 6)){
            char buff[128],buff_2[128];
            sprintf(buff,"%s.mat",ARG->file_C);
            ecriture(mat_C,size,size,buff);
            sprintf(buff_2,"%s.txt",ARG->file_C);
            ecriture_txt(mat_C,size,size,buff_2);
        }
        
        free(mat_A);
        free(mat_B);
        free(mat_C);
    }
    free(A_short);
    free(C_short);


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