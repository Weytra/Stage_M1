#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <string.h>


#include "../include/headers/lecture_mat.h"
#include "../include/headers/resize.h"
#include "../include/headers/gest_arg.h"

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
    position my_pos = get_position(my_rank,lg_grid);


    
    if(ARG->mode == 2){
        MPI_Bcast(&size,1,INT,0,MCW);
        dim_blk = size/lg_grid;
    }
    // -- Alocation memoire
    local_A = malloc(dim_blk*dim_blk*sizeof(int));
    local_B = malloc(dim_blk*dim_blk*sizeof(int));
    local_C = calloc(dim_blk*dim_blk,sizeof(int));
    using_A = malloc(dim_blk*dim_blk*sizeof(int));

    if(my_rank == 0){//allocation des donnes pour p0
        matrix_param * M_P = init_p0(ARG,lg_grid,1);
        A = M_P->matrix_A;
        B = M_P->matrix_B;
        A_blk = M_P->A_blk;
        B_blk = M_P->B_blk;
        C = M_P->matrix_C;
        tmp_C = M_P->matrix_tmp_C;
        size = M_P->size;
        old_size = M_P->old_size;
        to_resize = M_P->to_resize;

        memcpy(local_A,A_blk[0],dim_blk*dim_blk);
        memcpy(local_B,B_blk[0],dim_blk*dim_blk);

        for(int i = 1;i<lg_grid*lg_grid;i++){
            MPI_Send(A_blk)
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

        //phase de calcul
        //ajout de quelques variables pour optimiser le calculs
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
        if((ARG->mode == 1 && argc == 5) ||(ARG->mode == 2 && argc == 6)){
            char buff[128];
            char buff2[128];

            sprintf(buff,"%s.mat",ARG->file_C);
            ecriture(C,size,size,buff);

            sprintf(buff2,"%s.txt",ARG->file_C);
            ecriture_txt(C,size,size,buff2);
        }
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
    free(ARG);


    MPI_Finalize();
    return 0;
}