#include "stability.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ======================================================
//  COMPARAISON MATRICES ENTIERES
// ======================================================
StabilityResultInt compare_matrices_int( int *A,  int *B, size_t n)
{
    StabilityResultInt R = {0, 0, 0};

    for (size_t i = 0; i < n*n; i++) {
        long long diff = llabs((long long)A[i] - (long long)B[i]);
        if (diff != 0) {
            R.diff_count++;
            if (diff > R.max_error) R.max_error = diff;
            R.sum_error += diff;
        }
    }
    return R;
}

// ======================================================
//  COMPARAISON MATRICES FLOTTANTES
// ======================================================
StabilityResultFloat compare_matrices_float( float *A,  float *B, size_t n)
{
    StabilityResultFloat R = {0.0, 0.0, 0.0, 0};

    for (size_t i = 0; i < n*n; i++) {
        double a = A[i];
        double b = B[i];
        double abs_err = fabs(a - b);

        if (abs_err > 0) {
            R.diff_count++;
            if (abs_err > R.max_abs_error) R.max_abs_error = abs_err;
            R.sum_abs_error += abs_err;

            double rel_err = abs_err / (fabs(a) + 1e-12);
            if (rel_err > R.max_rel_error) R.max_rel_error = rel_err;
        }
    }
    return R;
}

void compararaison_int(char * mat_A, char * mat_B){
    Matrix A = lecture(mat_A);
    Matrix B = lecture(mat_B);

    if (!A.data || !B.data || A.cols != B.cols || A.rows != B.rows) {
        fprintf(stderr,"Erreur de chargement ou tailles incompatibles\n");
        return;
    }

    StabilityResultInt R = compare_matrices_int(A.data, B.data, A.cols);
    printf("=== Analyse stabilité (int) ===\n");
    printf("Différences : %lld\n", R.diff_count);
    printf("Erreur max : %lld\n", R.max_error);
    printf("Erreur totale : %lld\n", R.sum_error);

    free(A.data);
    free(B.data);
    return;
}

void compararaison_float(char *mat_A, char *mat_B)
{
    Matrix_f A = lecture_float(mat_A);
    Matrix_f B = lecture_float(mat_B);

    if (!A.data || !B.data || A.cols != B.cols || A.rows != B.rows) {
        fprintf(stderr,"Erreur de chargement ou tailles incompatibles\n");
        return;
    }

    StabilityResultFloat R = compare_matrices_float(A.data, B.data, A.cols);

    printf("=== Analyse stabilité (float) ===\n");
    printf("Différences : %lld\n", R.diff_count);
    printf("Erreur absolue max : %e\n", R.max_abs_error);
    printf("Erreur relative max : %e\n", R.max_rel_error);
    printf("Erreur absolue totale : %e\n", R.sum_abs_error);

    free(A.data);
    free(B.data);
    return ;
}
