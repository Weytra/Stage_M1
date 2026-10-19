#include "../headers/for_fox.h" 
#include <stdio.h>
#include <stdlib.h>


position get_position(int rank,int size){
    position pos = {rank/size,rank%size};
    return pos;
}

int get_id_proc(position pos,int size){
    return pos.ligne*size+pos.colone;
}

int**** to_blocking_dbl(int *A, int *B, int lg_grid, int dim_blk, int size){
    int**** Res = malloc(2 * sizeof(int***));

    int ***A_blk = malloc(lg_grid * sizeof(int**));
    int ***B_blk = malloc(lg_grid * sizeof(int**));

    for(int i = 0; i < lg_grid; i++){
        A_blk[i] = malloc(lg_grid * sizeof(int*));
        B_blk[i] = malloc(lg_grid * sizeof(int*));
        for(int j = 0; j < lg_grid; j++){
            A_blk[i][j] = malloc(dim_blk * dim_blk * sizeof(int));
            B_blk[i][j] = malloc(dim_blk * dim_blk * sizeof(int));
        }
    }

    for(int i = 0;i<lg_grid;i++){
        for(int j= 0;j<lg_grid;j++){
            for(int k = 0;k<dim_blk;k++){
                for(int p = 0;p<dim_blk;p++){
                    int row = i*dim_blk+k;
                    int col = j*dim_blk+p ;

                    A_blk[i][j][k*dim_blk+p] = A[row*size + col];
                    B_blk[i][j][k*dim_blk+p] = B[row*size + col];
                }
            }
        }
    }
    Res[0] = A_blk;
    Res[1] = B_blk;
    return Res;
}

int** to_blocking_dbl_lin(int *A, int *B, int lg_grid, int dim_blk, int size){
    int **Res = malloc(2 * sizeof(int*));
    int block_size = dim_blk * dim_blk;
    int nb_blocks = lg_grid * lg_grid;

    int *A_blk = malloc(nb_blocks * block_size * sizeof(int));
    int *B_blk = malloc(nb_blocks * block_size * sizeof(int));

    for(int i = 0;i<lg_grid;i++){
        for(int j= 0;j<lg_grid;j++){
            for(int k = 0;k<dim_blk;k++){
                for(int p = 0;p<dim_blk;p++){
                    int row = i*dim_blk+k;
                    int col = j*dim_blk+p ;

                    A_blk[(i*lg_grid+j)*dim_blk*dim_blk + k*dim_blk+p] = A[row*size + col];
                    B_blk[(i*lg_grid+j)*dim_blk*dim_blk + k*dim_blk+p] = B[row*size + col];
 
                }
            }
        }
    }
    Res[0] = A_blk;
    Res[1] = B_blk;
    return Res;
}



int** to_blocking_dbl_lin_t(int *A, int *B, int lg_grid, int dim_blk, int size){
    int **Res = malloc(2 * sizeof(int*));
    int *A_blk = malloc(size*size* sizeof(int));
    int *B_blk = malloc(size * size * sizeof(int));

    int****tmp =  to_blocking_dbl(A,B, lg_grid,dim_blk,size);

    for(int i = 0;i<lg_grid;i++){
        for(int j = 0;j<lg_grid;j++){
            for(int k = 0;k<dim_blk*dim_blk;k++){
                A_blk[ (i*lg_grid+j)*dim_blk*dim_blk +k]= tmp[0][i][j][k];
                B_blk[ (i*lg_grid+j)*dim_blk*dim_blk+k]= tmp[1][i][j][k];

            }
        }
    }
    
    Res[0] = A_blk;
    Res[1] = B_blk;
    return Res;
}

void reorganize(int * tmp_C,int * C,int size,int lg_grid,int dim_blk){
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
}

