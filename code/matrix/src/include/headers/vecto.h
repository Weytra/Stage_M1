#ifndef __VECTORISE__
#define __VECTORISE

#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

void matmul_block_avx512_gather(int *A, int *B, int *C, int size, int block);
void matmul_avx512_gather(int *A, int *B, int *C, int size);
void matmul_block_avx512_transposed(int *A, int *B, int *C, int size, int block);
void matmul_avx512_transposed(int *A, int *B, int *C, int size);
void transpose_B(int *B, int *B_T, int size);

void matmul_block_avx512_gather_omp(int *A, int *B, int *C, int size, int block);
void matmul_avx512_gather_omp(int *A, int *B, int *C, int size);

void matmul_ref(int *A, int *B, int *C, int size);


#endif 