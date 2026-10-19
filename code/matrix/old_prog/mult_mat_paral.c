#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>

#include "affichage.h"



#define MCW MPI_COMM_WORLD
#define MSI MPI_STATUS_IGNORE




//  C(m,p) = A(m,n) * B(n,p)
int main(int argc, char ** argv){
    int size;//taille des matrices

    if(argc == 1){
        size = 8;
    }
    if(argc == 2){
        size = atoi(argv[1]);
    }

    int *mat_A;
    int *mat_B;
    int *mat_C;
    int * A_short;
    int * B_short;
    int * C_short;


    int comm_size,my_rank;
    int size_b_p;//size by proc
    int block;

    srand(time(NULL));

    MPI_Init(NULL,NULL);

    MPI_Comm_size(MCW,&comm_size);
    MPI_Comm_rank(MCW,&my_rank);

    if (size % comm_size != 0) {
    if (my_rank == 0)
        fprintf(stderr, "Erreur: size (%d) doit être multiple de comm_size (%d)\n", size, comm_size);
    MPI_Abort(MCW, 1);
}


    size_b_p = size/comm_size * size;
    block = size/comm_size;


    if(!my_rank){
        mat_A = malloc(size * size * sizeof(int));
        mat_C = malloc(size * size * sizeof(int));
        mat_B = malloc(size * size * sizeof(int));
        for(int i = 0;i<size*size;i++){
            mat_B[i] = size*size - i;//rand()%1000;
        }
        for(int i = 0;i<size;i++){
            for(int j = 0;j<size;j++){
                (i == j) ? mat_A[i*size+j] = 1 : (mat_A[i*size+j] = 0);
            }
        }
        // printf("mat A : \n");
        // aff_mat(mat_A,size);
        // printf("mat B : \n");

        // aff_mat(mat_B,size);
    }
    A_short = malloc(size_b_p*sizeof(int));
    B_short = malloc(size_b_p*sizeof(int));
    C_short = calloc(size_b_p,sizeof(int));
    

    MPI_Scatter(mat_A, size_b_p, MPI_INT,A_short, size_b_p, MPI_INT,0, MCW);
    MPI_Scatter(mat_B, size_b_p,MPI_INT,B_short, size_b_p, MPI_INT,0, MCW);
    
    //on considere que la matrice B est en realité la transposé de la matrice reelle
    //calculs
    // int res;
    for (int i = 0;i<comm_size;i++){//pour parcourir tout le tabeau par paquet de size/commsize
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
        //on fait tourner le vecteur B_short 
        //chaque proc recupere le vecteur a sa droite(%) a chaque
        if(size > 48){// pour eviter le deadlock qui survient pour des grosse matrice
            if(my_rank%2){                
                MPI_Send(B_short,size_b_p,MPI_INT,(my_rank-1+comm_size)%comm_size,0,MCW);
                MPI_Recv(B_short,size_b_p,MPI_INT,(my_rank+1)%comm_size,0,MCW,MSI);
            }
            else{
                MPI_Recv(B_short,size_b_p,MPI_INT,(my_rank+1)%comm_size,0,MCW,MSI);
                MPI_Send(B_short,size_b_p,MPI_INT,(my_rank-1+comm_size)%comm_size,0,MCW);
            }
        }
        else{
            MPI_Send(B_short,size_b_p,MPI_INT,(my_rank-1+comm_size)%comm_size,0,MCW);
            MPI_Recv(B_short,size_b_p,MPI_INT,(my_rank+1)%comm_size,0,MCW,MSI);
        }
    }
    aff_vect(C_short,size_b_p,my_rank);

    MPI_Gather(C_short,size_b_p,MPI_INT,mat_C,size_b_p,MPI_INT,0,MCW);


    if(my_rank == 0){
        sleep(1);
        aff_mat(mat_C,size);
    }

    if (my_rank == 0) {
        free(mat_A);
        free(mat_B);
        free(mat_C);
    }
    free(A_short);
    free(B_short);
    free(C_short);

    MPI_Finalize();



    return 0;
}