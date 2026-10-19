#ifndef __STABILITY__
#define __STABILITY__

#include <stddef.h>
#include "lecture_mat.h"

// --- Pour matrices entières ---
typedef struct {
    long long max_error;
    long long sum_error;
    long long diff_count;
} StabilityResultInt;

StabilityResultInt compare_matrices_int(int *A, int *B, size_t n);

// --- Pour matrices flottantes ---
typedef struct {
    double max_abs_error;
    double max_rel_error;
    double sum_abs_error;
    long long diff_count;
} StabilityResultFloat;

StabilityResultFloat compare_matrices_float(float *A, float *B, size_t n);
void compararaison_int(char * mat_A, char * mat_B);
void compararaison_float(char * mat_A, char * mat_B);


#endif
