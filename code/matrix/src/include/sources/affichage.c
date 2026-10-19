#include <stdio.h>
#include "../headers/affichage.h"

void aff_mat(int *mat,int size){
    for(int i =0;i<size;i++){
        for(int j = 0;j<size;j++){
            printf("%d, ",mat[i*size +j]);
        }
        printf("\n");
    }
    printf("\n");
}

void aff_vect(int * mat, int taille,int my_rank){
    printf("proc %d :\n",my_rank);
    for(int i = 0;i<taille;i++)
        printf("%d ",mat[i]);
    printf("\n");
}

void aff_mat_blk(int *** mat, int size_mat,int size_blk){
    for(int i = 0;i<size_mat;i++){
        for(int j = 0;j<size_mat;j++){
            printf("block : %d, %d: \n",i,j);
            aff_mat(mat[i][j],size_blk);
        }
    }
}
