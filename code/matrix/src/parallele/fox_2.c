#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
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
#define INT MPI_INT

int main(int argc, char ** argv){

    //----initiallisation des variables----//

    arg * ARG = parse_arg(argc,argv);
    int size = ARG->size;

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

    int * A;
    int * B;
    int * C;
    int * local_A;
    int * local_B;
    int * local_C;
    int * tmp_C = NULL;
    int * using_A;
    int *** A_blk;
    int *** B_blk;
    
    int lg_grid = sqrt(comm_size);
    int dim_blk = size/lg_grid;
    int old_size = size,to_resize;
    position my_pos = get_position(my_rank,lg_grid);

    if(my_rank == 0){//allocation des donnes pour p0
        if(ARG->mode == 1){
            printf("old size : %d\n",old_size);
            A = malloc(old_size*old_size*sizeof(int));
            B = calloc(old_size*old_size,sizeof(int));
            
            //initialisation des donnees
            for(int i = 0;i<old_size;i++){
                for(int j = 0;j<old_size;j++){
                    A[i*old_size+j] = i*old_size+j+1;
                }
                B[i*old_size +i] = 1;//matrice identité
            }

            to_resize = (lg_grid - old_size%lg_grid)%lg_grid;//indique le nombre de ligne manquante (on reste sur des matrice carrées)
        
            size = old_size+to_resize;
            if (to_resize){
                A = resize(A,old_size,old_size,size,size);
                B = resize(B,old_size,old_size,size,size);
            }
        }
        else{
            Matrix M_A = lecture(ARG->file_A);
            Matrix M_B = lecture(ARG->file_B);
            A = M_A.data;
            B = M_B.data;

            old_size = max(M_A.cols,M_A.rows,M_B.cols,M_B.rows);
            
            to_resize = (lg_grid - old_size%lg_grid)%lg_grid;//indique le nombre de ligne manquante (on reste sur des matrice carrées)
        
            size = old_size+to_resize;

            if(size != old_size || size != min(M_A.cols,M_A.rows,M_B.cols,M_B.rows)){
                A = resize(A,M_A.rows,M_A.cols,size,size);
                B = resize(B,M_B.rows,M_B.cols,size,size);
            }
        }
        
        C = calloc(size*size,sizeof(int));
        tmp_C = malloc(size*size*sizeof(int));

        dim_blk = size/lg_grid;
        // -- Preparation de la matrice pour l'envoie
        int **** res = to_blocking_dbl(A,B,lg_grid,dim_blk,size);
        A_blk = res[0];
        B_blk = res[1];
        free(res);  
        //envoie des block de matrices au processus.
    }

    MPI_Bcast(&size,1,INT,0,MCW);
    MPI_Bcast(&to_resize,1,INT,0,MCW);
    dim_blk = size/lg_grid;

    // -- Alocation memoire
    local_A = malloc(dim_blk*dim_blk*sizeof(int));
    local_B = malloc(dim_blk*dim_blk*sizeof(int));
    local_C = calloc(dim_blk*dim_blk,sizeof(int));
    using_A = malloc(dim_blk*dim_blk*sizeof(int));

    dim_blk = size/lg_grid;
    
    MPI_Barrier(MCW);
    double algo_start_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[0] = read_energy_uj();

    if(!my_rank){
        for(int i = 0;i<lg_grid;i++){
            for(int j = 0;j<lg_grid;j++){
                int recv = (i*lg_grid+j);
                if(recv != my_rank){
                    MPI_Send(A_blk[i][j],dim_blk*dim_blk,INT,(i*lg_grid+j),0,MCW);
                    MPI_Send(B_blk[i][j],dim_blk*dim_blk,INT,(i*lg_grid+j),0,MCW);
                }
                else{
                    memcpy(local_A, A_blk[i][j], dim_blk*dim_blk*sizeof(int));
                    memcpy(local_B, B_blk[i][j], dim_blk*dim_blk*sizeof(int));
                }
            }
        }
    }
    //---- Fin du travail de p0-----
    

    if(my_rank){//tout le monde recois sauf p0 qui a deja fait sa propre copie
        MPI_Recv(local_A,dim_blk*dim_blk,INT,0,0,MCW,MSI);
        MPI_Recv(local_B,dim_blk*dim_blk,INT,0,0,MCW,MSI);
    }
    //_______________________________________________
    //  fin de la distibution initiale des matrices
    //_______________________________________________

    for(int i =0;i<lg_grid;i++){
        //gather lineaire d'une des sous matrice A
        position root = {my_pos.ligne,(my_pos.ligne + i) % lg_grid};
        int root_id = get_id_proc(root, lg_grid);
        //verifier la ligne precedente pour des taille superieur a 4 proc
        if (root_id == my_rank){
            for(int p = 0;p<lg_grid;p++){
                if (p!= root.colone){
                    position dest = {my_pos.ligne,p};
                    int dst_id = get_id_proc(dest,lg_grid);
                    MPI_Send(local_A,dim_blk*dim_blk,INT,dst_id,0,MCW);
                }
            }
            memcpy(using_A, local_A, dim_blk*dim_blk*sizeof(int));
        }
        else{
            MPI_Recv(using_A,dim_blk*dim_blk,INT,root_id,0,MCW,MSI);
        }

        //calcul
        if(!ARG->block){
            for(int k = 0;k<dim_blk;k++){
                int rowA = k*dim_blk;
                for(int m = 0;m<dim_blk;m++){
                    int idc  = rowA+m;
                    int sum = local_C[idc];
                    for(int p = 0;p<dim_blk;p++){
                        sum += local_A[rowA+p]*local_B[p*dim_blk+m];
                    }
                    local_C[idc] = sum;
                }
            }
        }
        else{
            int block = ARG->block;
            for(int k = 0;k<dim_blk;k+=block){
                int kk_end = (k + block < dim_blk)? k + block : dim_blk;
                for(int kk = k;kk<kk_end;kk++){
                    int rowA = kk*dim_blk;
    
                    for(int m = 0;m<dim_blk;m+=block){
                        int mm_end = (m + block < dim_blk)? m + block : dim_blk;
                        for(int mm = m;mm<mm_end;mm++){
                            int idc  = rowA+mm;
                            int sum = local_C[idc];

                            for(int p = 0;p<dim_blk;p+=block){
                                int pp_end = (p + block < dim_blk)? p + block : dim_blk;
                                for(int pp = p;pp<pp_end;pp++){
                                    sum += local_A[rowA+pp]*local_B[pp*dim_blk+mm];
                                }
                            }
                            local_C[idc] = sum;
                        }
                    }
                }
            }            
        }

        // tournage des matrice B
        position tmp_id;
        tmp_id.ligne = (my_pos.ligne+1)%lg_grid;
        tmp_id.colone = my_pos.colone;
        int recv = get_id_proc(tmp_id,lg_grid);
        tmp_id.ligne = (my_pos.ligne-1+lg_grid)%lg_grid;
        tmp_id.colone = my_pos.colone;
        int send = get_id_proc(tmp_id,lg_grid);
        MPI_Sendrecv_replace(local_B,
                     dim_blk * dim_blk, INT,
                     recv, 0,
                     send, 0,
                     MCW, MSI);
    }

    //on fait un gather de tout ce beau monde
    MPI_Gather(local_C,dim_blk*dim_blk,INT,tmp_C,dim_blk*dim_blk,INT,0,MCW);
    MPI_Barrier(MCW);
    
    double algo_end_time = MPI_Wtime();
    if(!my_rank)
        algo_energy[1] = read_energy_uj();
    //maintenant les resultat sans dans tmpC mais pas dans le bon ordre car la
    //matrice est sous forme "blocké" il faut donc la remetre dans sa forme initiale
    if(!my_rank){
        for(int p = 0;p<lg_grid;p++){
            for(int q = 0;q<lg_grid;q++){
                for(int r = 0;r<dim_blk;r++){
                    for(int s = 0;s<dim_blk;s++){
                        int lg = p*dim_blk+r;
                        int col = q*dim_blk+s;
                        C[lg*size+col] = tmp_C[(p*lg_grid+q)*dim_blk*dim_blk  +
                                                 r * dim_blk+s];
                    }
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

        for(int i =0;i<size*size;i++){
            if(A[i] != C[i])
                printf("cest la D !!!!\n");
        }
        //aff_mat(C,size);
    }



    // -- netoyage de la memoire --
    if(!my_rank){ //pour le processus maitre
        free(A);
        free(B);
        free(C);
        free(tmp_C);
        for(int a=0;a<lg_grid;a++){
            for(int b = 0;b<lg_grid;b++){
                free(A_blk[a][b]);
                free(B_blk[a][b]);
            }
            free(A_blk[a]);
            free(B_blk[a]);
        }
        free(A_blk);
        free(B_blk);
    }
    // pour tout le monde
    free(local_A);
    free(local_B);
    free(local_C);
    free(using_A);

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
