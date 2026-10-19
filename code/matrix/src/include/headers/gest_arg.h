#ifndef __ARG__
#define __ARG__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "lecture_mat.h"
#include "for_fox.h"
#include "resize.h"

typedef struct arg{
    int size;
    int block;
    int mode;
    char * file_A;
    char * file_B;
    char * file_C;
}arg;

typedef struct matrix_param{
    int * matrix_A;
    int * matrix_B;
    int * matrix_C;
    int * A_blk;
    int * B_blk;
    int * matrix_tmp_C;
    int size;
    int old_size;
    int to_resize;
} matrix_param;


arg * parse_arg(int argc, char ** argv);
matrix_param * init_p0(arg * ARG,int lg_grid,short need_tmp_C,short need_blocking);


#endif