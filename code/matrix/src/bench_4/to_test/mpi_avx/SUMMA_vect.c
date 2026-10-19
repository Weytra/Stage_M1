#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <string.h>


#include "../../../include/headers/affichage.h"
#include "../../../include/headers/lecture_mat.h"
#include "../../../include/headers/resize.h"
#include "../../../include/headers/gest_arg.h"
#include "../../../include/headers/energy_monitor.h"
#include "../../../include/headers/vecto.h"



#define MCW MPI_COMM_WORLD
#define MSI MPI_STATUS_IGNORE
#define INT MPI_INT

int main(int argc,char ** argv){

    arg * ARG = parse_arg(argc,argv);

    int my_rank,comm_size;
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
        
    //________________________________________________
    // --- CREATION DE LA GRILLE DE PROCESSUS MPI ---
    //________________________________________________
    MPI_Comm grid_comm;
    int lg_grid = sqrt(comm_size);
    int dims[2] = {lg_grid, lg_grid};
    int periods[2] = {1,1}; // torique sur les deux dimensions
    int reorder = 0;         // laisse pas MPI réordonner les ranks pour optimiser

    MPI_Cart_create(MCW, 2, dims, periods, reorder, &grid_comm);

    //_____________________________________________________
    // ---- CREATION DES COMMUNICATION PAR LIGNE/COLONES
    //_____________________________________________________

    MPI_Comm row_comm,col_comm;
    int remains_dims[2] = {0,1};
    MPI_Cart_sub(grid_comm,remains_dims,&row_comm);
    remains_dims[0] = 1;
    remains_dims[1] = 0;
    MPI_Cart_sub(grid_comm,remains_dims,&col_comm);

    int my_rank_cart;
    MPI_Comm_rank(grid_comm, &my_rank_cart);

    int coords[2];
    MPI_Cart_coords(grid_comm, my_rank_cart, 2, coords);

    int my_row = coords[0];
    int my_col = coords[1];

    //_____________________________________________________
    // ---- FIN INITALISATION STRUCTURE MPI ----
    //_____________________________________________________

    int * A;
    int * B;
    int * C;
    int * local_A;
    int * local_B; 
    int * local_C;
    int * tmp_C = NULL;
    int * A_blk;
    int * B_blk;
    int * using_A;
    int * using_B;
    
    int size = ARG->size;
    int dim_blk;
    int old_size;
    int to_resize;
    if(ARG->mode ==1){
        dim_blk = size/lg_grid;
    }
    if(my_rank == 0){//allocation des donnes pour p0
        matrix_param * M_P = init_p0(ARG,lg_grid,1,1);
        A = M_P->matrix_A;
        B = M_P->matrix_B;
        
        A_blk = M_P->A_blk;
        B_blk = M_P->B_blk;
        C = M_P->matrix_C;
        tmp_C = M_P->matrix_tmp_C;
        size = M_P->size;
        old_size = M_P->old_size;
        to_resize = M_P->to_resize;
    }


    MPI_Bcast(&size,1,INT,0,MCW);
    dim_blk = size/lg_grid;
    
    local_A = malloc(dim_blk*dim_blk*sizeof(int));
    local_B = malloc(dim_blk*dim_blk*sizeof(int));
    local_C = calloc(dim_blk*dim_blk,sizeof(int));
    using_A = malloc(dim_blk*dim_blk * sizeof(int));
    using_B = malloc(dim_blk*dim_blk * sizeof(int));
    
    MPI_Barrier(MCW);

    double algo_start_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[0] = read_energy_uj();

    MPI_Scatter(A_blk, dim_blk*dim_blk,INT,
        local_A, dim_blk*dim_blk, INT,
        0, grid_comm);
        
    MPI_Scatter(B_blk, dim_blk*dim_blk,INT,
        local_B, dim_blk*dim_blk, INT,
        0, grid_comm);

    transpose(local_B,dim_blk);


            
    for(int i = 0;i<lg_grid;i++){
        if(my_col == i){
            memcpy(using_A,local_A,dim_blk*dim_blk*sizeof(int));
        }
        MPI_Bcast(using_A,dim_blk*dim_blk,INT,i,row_comm);

        if(my_row == i){
            memcpy(using_B,local_B,dim_blk*dim_blk*sizeof(int));
        }
        
        MPI_Bcast(using_B,dim_blk*dim_blk,INT,i,col_comm);


        if (ARG->block) {
            matmul_block_avx512_transposed(using_A, using_B, local_C, dim_blk, ARG->block);
        } else {
            matmul_avx512_transposed(using_A, using_B, local_C, dim_blk);
        }
    }
    
    MPI_Gather(local_C,dim_blk*dim_blk,INT,tmp_C,dim_blk*dim_blk,INT,0,MCW);
    MPI_Barrier(MCW);
    
    double algo_end_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[1] = read_energy_uj();

    if(!my_rank){
        reorganize(tmp_C,C,size,lg_grid,dim_blk);
        
        if(ARG->mode == 1 && argc == 5 || (ARG->mode ==2 && argc == 6)){
            char buff[128],buff_2[128];
            sprintf(buff,"%s.mat",ARG->file_C);
            ecriture(C,size,size,buff);
            sprintf(buff_2,"%s.txt",ARG->file_C);
            ecriture_txt(C,size,size,buff_2);
        }
        printf("Execution termine avec succes\n");
    }

    // -- netoyage de la memoire --
    if(!my_rank){ //pour le processus maitre
        free(A);
        free(B);
        free(C);
        free(tmp_C);
        free(A_blk);
        free(B_blk);
    }
    // pour tout le monde
    free(local_A);
    free(local_B);
    free(local_C);
    free(using_B);
    free(using_A);
    free(ARG);


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