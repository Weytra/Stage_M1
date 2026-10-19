#include "../headers/resize.h"

int * resize(int *mat, int act_col,int act_lg,int new_lg,int new_col){
    int * new_mat = calloc(new_col*new_lg,sizeof(int));
        
    for(int i = 0; i < act_lg; i++) {
        memcpy(&new_mat[i * new_col],&mat[i * act_col],act_col * sizeof(int));
    }

    free(mat);
    return new_mat;
}

int max(int a, int b, int c, int d){
    int m = a;

    if (b > m) m = b;
    if (c > m) m = c;
    if (d > m) m = d;

    return m;
}   

int min(int a, int b, int c, int d){
    int m = a;

    if (b < m) m = b;
    if (c < m) m = c;
    if (d < m) m = d;

    return m;
}   
