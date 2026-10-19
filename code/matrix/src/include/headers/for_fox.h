#ifndef __FOX__
#define __FOX__

#include "lecture_mat.h"
#include "gest_arg.h"

typedef struct position{
    int ligne,colone;
}position;

position get_position(int rank,int size);
int get_id_proc(position pos,int size);

void aff_mat_blk(int *** mat, int size_mat,int size_blk);
void aff_vect(int * mat, int taille,int my_rank);
void aff_mat(int *mat,int size);

int **** to_blocking_dbl(int * A,int * B,int lg_grid,int dim_blk,int size);
int** to_blocking_dbl_lin(int *A, int *B, int lg_grid, int dim_blk, int size);
int** to_blocking_dbl_lin_t(int *A, int *B, int lg_grid, int dim_blk, int size);

void reorganize(int * tmp_C,int * C,int size,int lg_grid,int dim_blk);


#endif