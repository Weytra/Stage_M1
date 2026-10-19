#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include<string.h>

#include "../include/headers/affichage.h"
#include "../include/headers/for_fox.h" 
#include "../include/headers/lecture_mat.h"
#include "../include/headers/resize.h"
#include "../include/headers/gest_arg.h"
#include "../include/headers/energy_monitor.h"



#define MCW MPI_COMM_WORLD
#define MSI MPI_STATUS_IGNORE
#define INT MPI_INT

int main(int argc, char ** argv){
    arg * ARG = parse_arg(argc,argv);
    int size = ARG->size;
    //srand(time(NULL));

    MPI_Init(NULL,NULL);
    int comm_size,my_rank;
    int size_b_p;//size by proc
    MPI_Comm_size(MCW,&comm_size);
    MPI_Comm_rank(MCW,&my_rank);


    //donne pour le benchmark
    long long global_energy[2];
    long long algo_energy[2];

    MPI_Barrier(MCW);
    double global_start_time = MPI_Wtime();
    if(!my_rank)
        global_energy[0] = read_energy_uj();

        

    if (size % comm_size != 0) {
        if (my_rank == 0)
            fprintf(stderr, "Erreur: size (%d) doit être multiple de comm_size (%d)\n", size, comm_size);
        MPI_Abort(MCW, 1);
    }

        int *mat_A;
    int *mat_B;
    int *mat_C;
    int * tmp_b;
    int * A_short;
    int * B_short;
    int * C_short;

    int block;
    int old_size,to_resize;

    
    if(!my_rank){
        matrix_param * M_P = init_p0(ARG,comm_size,0,0);//pas besoin de tmpc ici ni de blocking
        mat_A = M_P->matrix_A;
        mat_B = M_P->matrix_B;
        
        mat_C = M_P->matrix_C;
        size = M_P->size;
        old_size = M_P->old_size;
        to_resize = M_P->to_resize;
    }
    
    MPI_Bcast(&size,1,INT,0,MCW);
    size_b_p = size/comm_size * size;
    block = size/comm_size;
    
    A_short = malloc(size_b_p*sizeof(int));
    B_short = malloc(size_b_p*sizeof(int));
    C_short = calloc(size_b_p,sizeof(int));  
    
    if(!(my_rank%2))  
        tmp_b = malloc(size_b_p*sizeof(int));    

    MPI_Barrier(MCW);

    double algo_start_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[0] = read_energy_uj();

    MPI_Scatter(mat_A, size_b_p, MPI_INT,A_short, size_b_p, MPI_INT,0, MCW);
    MPI_Scatter(mat_B, size_b_p,MPI_INT,B_short, size_b_p, MPI_INT,0, MCW);
    
    //on considere que la matrice B est en realité la transposé de la matrice reelle
    //calculs
    int local_rows = size / comm_size;
    for (int i = 0;i<comm_size;i++){//pour parcourir tout le tabeau par paquet de size/commsize
        if(ARG->block){
            int blk = ARG->block;   
            
            for(int p = 0;p<block;p+=blk){//parcour de chaque ligne de la division
                for(int q = 0;q<block;q+=blk){//parcour de chaque ligne de la division
                    for(int j = 0;j<size;j+=blk){//parcour de la ligne complete
                        int j_max = (j + blk > size) ? size : j + blk;
                        int q_max = (q + blk > local_rows) ? local_rows : q + blk;
                        int p_max = (p + blk > local_rows) ? local_rows : p + blk;

                        for(int pp = p ; pp<p_max;pp++){
                            for(int qq = q;qq<q_max;qq++){
                                int col_block = ((i + my_rank) % comm_size) * block;
                                int row = pp * size;
                                int col = col_block + qq;
                                int res = C_short[row + col];
                                
                                for(int jj = j;jj<j_max;jj++){
                                    res += A_short[pp*size+jj] * B_short[qq*size+jj];
                                }
                                
                                C_short[row + col] = res;
                            }
                        }
                    }
                    // //C_short[(((i+my_rank)%comm_size*block)) +(size*p)+q] = res;
                    // int col_block = ((i + my_rank) % comm_size) * block;
                    // int row = p * size;
                    // int col = col_block + q;
                    // C_short[row + col] = res;

                }
            }
        }
        else{
            for(int p = 0;p<block;p++){//parcour de chaque ligne de la division
                for(int q = 0;q<block;q++){//parcour de chaque ligne de la division
                    
                    int res = 0;
                    for(int j = 0;j<size;j++){//parcour de la ligne complete
                        res += A_short[p*size+j] * B_short[q*size+j];
                    }
                    // printf("proc_%d  it %d : p = %d    q = %d : res = %d\n",my_rank,i,p,q,res);
                    
                    //C_short[(((i+my_rank)%comm_size*block)) +(size*p)+q] = res;
                    int global_col = ((my_rank + i) % comm_size) * block + q;
                    C_short[p*size + global_col] = res;

                }
            }
        }
        //on fait tourner le vecteur B_short 
        //chaque proc recupere le vecteur a sa droite(%) a chaque
        if(my_rank%2){                
            MPI_Send(B_short,size_b_p,MPI_INT,(my_rank-1+comm_size)%comm_size,0,MCW);
            MPI_Recv(B_short,size_b_p,MPI_INT,(my_rank+1)%comm_size,0,MCW,MSI);
        }
        else{//!\ tester aussi avec la meme forme pour les proc impair pour la symetrie
            MPI_Recv(tmp_b,size_b_p,MPI_INT,(my_rank+1)%comm_size,0,MCW,MSI);
            MPI_Send(B_short,size_b_p,MPI_INT,(my_rank-1+comm_size)%comm_size,0,MCW);
            int *tmp = B_short;
            B_short = tmp_b;
            tmp_b = tmp;
        }
    }

    MPI_Gather(C_short,size_b_p,MPI_INT,mat_C,size_b_p,MPI_INT,0,MCW);
    MPI_Barrier(MCW);
    
    double algo_end_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[1] = read_energy_uj();

    // if(my_rank == 0)
    //     aff_mat(mat_C,size);

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
    free(B_short);
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