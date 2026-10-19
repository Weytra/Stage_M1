#include "../headers/vecto.h"

void matmul_block_avx512_gather(int *A, int *B, int *C, int size, int block) {
    for (int ii = 0; ii < size; ii += block) {
        int i_end = (ii + block < size) ? ii + block : size;

        for (int jj = 0; jj < size; jj += block) {
            int j_end = (jj + block < size) ? jj + block : size;

            for (int kk = 0; kk < size; kk += block) {
                int k_end = (kk + block < size) ? kk + block : size;

                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {

                        __m512i vsum = _mm512_setzero_si512();
                        int k;

                        // Partie vectorisée
                        for (k = kk; k + 16 <= k_end; k += 16) {

                            // A[i, k..k+15] contigu
                            __m512i va = _mm512_loadu_si512(
                                (__m512i*)&A[i*size + k]
                            );

                            // Indices pour B[k..k+15, j] (colonne j)
                            __m512i idx = _mm512_set_epi32(
                                (k+15)*size + j, (k+14)*size + j,
                                (k+13)*size + j, (k+12)*size + j,
                                (k+11)*size + j, (k+10)*size + j,
                                (k+9)*size + j,  (k+8)*size + j,
                                (k+7)*size + j,  (k+6)*size + j,
                                (k+5)*size + j,  (k+4)*size + j,
                                (k+3)*size + j,  (k+2)*size + j,
                                (k+1)*size + j,  (k+0)*size + j
                            );

                            __m512i vb = _mm512_i32gather_epi32(idx, B, 4);

                            __m512i vmul = _mm512_mullo_epi32(va, vb);
                            vsum = _mm512_add_epi32(vsum, vmul);
                        }

                        // Réduction horizontale
                        int partial[16];
                        _mm512_storeu_si512((__m512i*)partial, vsum);

                        int sum = 0;
                        for (int t = 0; t < 16; t++)
                            sum += partial[t];

                        // Reste scalaire
                        for (; k < k_end; k++) {
                            sum += A[i*size + k] * B[k*size + j];
                        }

                        C[i*size + j] += sum;
                    }
                }
            }
        }
    }
}

void matmul_avx512_gather(int *A, int *B, int *C, int size)
{
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            __m512i vsum = _mm512_setzero_si512();
            int k;

            // Boucle vectorisée par pas de 16
            for (k = 0; k + 16 <= size; k += 16) {

                // A[i, k..k+15] contigu
                __m512i va = _mm512_loadu_si512(
                    (__m512i*)&A[i*size + k]
                );

                // Indices pour B[k..k+15, j] (colonne j)
                __m512i idx = _mm512_set_epi32(
                    (k+15)*size + j, (k+14)*size + j,
                    (k+13)*size + j, (k+12)*size + j,
                    (k+11)*size + j, (k+10)*size + j,
                    (k+9)*size + j,  (k+8)*size + j,
                    (k+7)*size + j,  (k+6)*size + j,
                    (k+5)*size + j,  (k+4)*size + j,
                    (k+3)*size + j,  (k+2)*size + j,
                    (k+1)*size + j,  (k+0)*size + j
                );

                // Gather de la colonne de B
                __m512i vb = _mm512_i32gather_epi32(idx, B, 4);

                // Multiplication + accumulation
                __m512i vmul = _mm512_mullo_epi32(va, vb);
                vsum = _mm512_add_epi32(vsum, vmul);
            }

            // Réduction horizontale
            int partial[16];
            _mm512_storeu_si512((__m512i*)partial, vsum);

            int sum = 0;
            for (int t = 0; t < 16; t++)
                sum += partial[t];

            // Reste scalaire
            for (; k < size; k++) {
                sum += A[i*size + k] * B[k*size + j];
            }

            C[i*size + j] = sum;
        }
    }
}


void transpose_B(int *B, int *B_T, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            B_T[j*size + i] = B[i*size + j];
        }
    }
}

void matmul_block_avx512_transposed(int *A, int *B, int *C, int size, int block) {
    for (int ii = 0; ii < size; ii += block) {
        int i_end = (ii + block < size) ? ii + block : size;

        for (int jj = 0; jj < size; jj += block) {
            int j_end = (jj + block < size) ? jj + block : size;

            for (int kk = 0; kk < size; kk += block) {
                int k_end = (kk + block < size) ? kk + block : size;

                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {

                        __m512i vsum = _mm512_setzero_si512();
                        int k;

                        // Partie vectorisée : A[i, k..k+15] et B_T[j, k..k+15] contigus
                        for (k = kk; k + 16 <= k_end; k += 16) {

                            __m512i va = _mm512_loadu_si512(
                                (__m512i*)&A[i*size + k]
                            );

                            __m512i vb = _mm512_loadu_si512(
                                (__m512i*)&B[j*size + k]
                            );

                            __m512i vmul = _mm512_mullo_epi32(va, vb);
                            vsum = _mm512_add_epi32(vsum, vmul);
                        }

                        int partial[16];
                        _mm512_storeu_si512((__m512i*)partial, vsum);

                        int sum = 0;
                        for (int t = 0; t < 16; t++)
                            sum += partial[t];

                        // Reste scalaire
                        for (; k < k_end; k++) {
                            sum += A[i*size + k] * B[j*size + k];
                        }

                        C[i*size + j] += sum;
                    }
                }
            }
        }
    }

}


void matmul_avx512_transposed(int *A, int *B, int *C, int size)
{
    // Triple boucle classique, mais vectorisée sur k
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            __m512i vsum = _mm512_setzero_si512();
            int k;

            // Boucle vectorisée par pas de 16
            for (k = 0; k + 16 <= size; k += 16) {

                // A[i, k..k+15] contigu
                __m512i va = _mm512_loadu_si512(
                    (__m512i*)&A[i*size + k]
                );

                // B_T[j, k..k+15] contigu
                __m512i vb = _mm512_loadu_si512(
                    (__m512i*)&B_T[j*size + k]
                );

                __m512i vmul = _mm512_mullo_epi32(va, vb);
                vsum = _mm512_add_epi32(vsum, vmul);
            }

            // Réduction horizontale
            int partial[16];
            _mm512_storeu_si512((__m512i*)partial, vsum);

            int sum = 0;
            for (int t = 0; t < 16; t++)
                sum += partial[t];

            // Reste scalaire
            for (; k < size; k++) {
                sum += A[i*size + k] * B[j*size + k];
            }

            C[i*size + j] = sum;
        }
    }

}

void matmul_ref(int *A, int *B, int *C, int size)
{
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            for (int k = 0; k < size; k++) {
                sum += A[i*size + k] * B[k*size + j];
            }
            C[i*size + j] = sum;
        }
    }
}

void matmul_block_avx512_gather_omp(int *A, int *B, int *C, int size, int block) {
    #pragma omp parallel for collapse(2)
    for (int ii = 0; ii < size; ii += block) {
        int i_end = (ii + block < size) ? ii + block : size;

        for (int jj = 0; jj < size; jj += block) {
            int j_end = (jj + block < size) ? jj + block : size;

            for (int kk = 0; kk < size; kk += block) {
                int k_end = (kk + block < size) ? kk + block : size;

                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {

                        __m512i vsum = _mm512_setzero_si512();
                        int k;

                        // Partie vectorisée
                        for (k = kk; k + 16 <= k_end; k += 16) {

                            // A[i, k..k+15] contigu
                            __m512i va = _mm512_loadu_si512(
                                (__m512i*)&A[i*size + k]
                            );

                            // Indices pour B[k..k+15, j] (colonne j)
                            __m512i idx = _mm512_set_epi32(
                                (k+15)*size + j, (k+14)*size + j,
                                (k+13)*size + j, (k+12)*size + j,
                                (k+11)*size + j, (k+10)*size + j,
                                (k+9)*size + j,  (k+8)*size + j,
                                (k+7)*size + j,  (k+6)*size + j,
                                (k+5)*size + j,  (k+4)*size + j,
                                (k+3)*size + j,  (k+2)*size + j,
                                (k+1)*size + j,  (k+0)*size + j
                            );

                            __m512i vb = _mm512_i32gather_epi32(idx, B, 4);

                            __m512i vmul = _mm512_mullo_epi32(va, vb);
                            vsum = _mm512_add_epi32(vsum, vmul);
                        }

                        // Réduction horizontale
                        int partial[16];
                        _mm512_storeu_si512((__m512i*)partial, vsum);

                        int sum = 0;
                        for (int t = 0; t < 16; t++)
                            sum += partial[t];

                        // Reste scalaire
                        for (; k < k_end; k++) {
                            sum += A[i*size + k] * B[k*size + j];
                        }

                        C[i*size + j] += sum;
                    }
                }
            }
        }
    }
}

void matmul_avx512_gather_omp(int *A, int *B, int *C, int size)
{
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            __m512i vsum = _mm512_setzero_si512();
            int k;

            // Boucle vectorisée par pas de 16
            for (k = 0; k + 16 <= size; k += 16) {

                // A[i, k..k+15] contigu
                __m512i va = _mm512_loadu_si512(
                    (__m512i*)&A[i*size + k]
                );

                // Indices pour B[k..k+15, j] (colonne j)
                __m512i idx = _mm512_set_epi32(
                    (k+15)*size + j, (k+14)*size + j,
                    (k+13)*size + j, (k+12)*size + j,
                    (k+11)*size + j, (k+10)*size + j,
                    (k+9)*size + j,  (k+8)*size + j,
                    (k+7)*size + j,  (k+6)*size + j,
                    (k+5)*size + j,  (k+4)*size + j,
                    (k+3)*size + j,  (k+2)*size + j,
                    (k+1)*size + j,  (k+0)*size + j
                );

                // Gather de la colonne de B
                __m512i vb = _mm512_i32gather_epi32(idx, B, 4);

                // Multiplication + accumulation
                __m512i vmul = _mm512_mullo_epi32(va, vb);
                vsum = _mm512_add_epi32(vsum, vmul);
            }

            // Réduction horizontale
            int partial[16];
            _mm512_storeu_si512((__m512i*)partial, vsum);

            int sum = 0;
            for (int t = 0; t < 16; t++)
                sum += partial[t];

            // Reste scalaire
            for (; k < size; k++) {
                sum += A[i*size + k] * B[k*size + j];
            }

            C[i*size + j] = sum;
        }
    }
}

void matmul_block_avx512_transposed_omp(int *A, int *B, int *C,
                                        int size, int block)
{
    // On suppose C initialisé à 0 avant l'appel,

    #pragma omp parallel
    {
        // Tout le monde partage A, B_T, C, size, block
        #pragma omp for collapse(2) schedule(static)
        for (int ii = 0; ii < size; ii += block) {
            for (int jj = 0; jj < size; jj += block) {

                for (int kk = 0; kk < size; kk += block) {
                    int i_end = (ii + block < size) ? ii + block : size;
                    int j_end = (jj + block < size) ? jj + block : size;
                    int k_end = (kk + block < size) ? kk + block : size;

                    for (int i = ii; i < i_end; i++) {
                        for (int j = jj; j < j_end; j++) {

                            __m512i vsum = _mm512_setzero_si512();
                            int k;

                            // Partie vectorisée
                            for (k = kk; k + 16 <= k_end; k += 16) {
                                __m512i va = _mm512_loadu_si512(
                                    (__m512i*)&A[i*size + k]
                                );
                                __m512i vb = _mm512_loadu_si512(
                                    (__m512i*)&B[j*size + k]
                                );
                                __m512i vmul = _mm512_mullo_epi32(va, vb);
                                vsum = _mm512_add_epi32(vsum, vmul);
                            }

                            int partial[16];
                            _mm512_storeu_si512((__m512i*)partial, vsum);

                            int sum = 0;
                            for (int t = 0; t < 16; t++)
                                sum += partial[t];

                            // Reste scalaire
                            for (; k < k_end; k++) {
                                sum += A[i*size + k] * B[j*size + k];
                            }

                            // Chaque (i,j) est traité par un seul thread
                            C[i*size + j] += sum;
                        }
                    }
                }
            }
        }
    }
}
