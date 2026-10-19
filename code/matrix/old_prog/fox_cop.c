#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

#include "../include/headers/for_fox.h"
#include "../include/headers/lecture_mat.h"

// Types et constantes
#define INT MPI_INT
#define MCW MPI_COMM_WORLD


void fox(int n, int lg_grid,
         int my_rank, position my_pos,
         int *local_A, int *local_B, int *local_C)
{
    int dim_blk = n / lg_grid;
    int blk_size = dim_blk * dim_blk;

    MPI_Status status;

    // init C local
    for (int i = 0; i < blk_size; i++)
        local_C[i] = 0;

    int *using_A = malloc(blk_size * sizeof(int));
    if (!using_A) {
        fprintf(stderr, "Rank %d: erreur malloc using_A\n", my_rank);
        MPI_Abort(MCW, 1);
    }

    for (int k = 0; k < lg_grid; k++) {

        // ---------- 1) Diffusion de A sur la ligne ----------
        position root_pos = { my_pos.ligne, (my_pos.ligne + k) % lg_grid };
        int root_id = get_id_proc(root_pos, lg_grid);

        if (my_rank == root_id) {
            // root envoie son bloc A à tous les autres de la ligne
            for (int c = 0; c < lg_grid; c++) {
                position dst_pos = { my_pos.ligne, c };
                int dst_id = get_id_proc(dst_pos, lg_grid);
                if (dst_id != my_rank) {
                    MPI_Send(local_A, blk_size, INT, dst_id, 0, MCW);
                }
            }
            // copie locale
            memcpy(using_A, local_A, blk_size * sizeof(int));
        } else {
            // les autres reçoivent le bloc A du root de la ligne
            MPI_Recv(using_A, blk_size, INT, root_id, 0, MCW, &status);
        }

        // ---------- 2) Calcul local C += using_A * local_B ----------
        for (int i = 0; i < dim_blk; i++) {
            for (int j = 0; j < dim_blk; j++) {
                int sum = 0;
                for (int l = 0; l < dim_blk; l++) {
                    int a = using_A[i * dim_blk + l];
                    int b = local_B[l * dim_blk + j];
                    sum += a * b;
                }
                local_C[i * dim_blk + j] += sum;
            }
        }

        // ---------- 3) Rotation de B dans la colonne ----------
        position send_pos = { (my_pos.ligne - 1 + lg_grid) % lg_grid, my_pos.colone };
        position recv_pos = { (my_pos.ligne + 1) % lg_grid, my_pos.colone };

        int send_id = get_id_proc(send_pos, lg_grid);
        int recv_id = get_id_proc(recv_pos, lg_grid);

        MPI_Sendrecv_replace(local_B,
                             blk_size, INT,
                             recv_id, 0,
                             send_id, 0,
                             MCW, &status);
    }

    free(using_A);
}

// ==== Reconstruction de C globale à partir des blocs ====

void gather_C(int *global_C, int *local_C,
              int n, int lg_grid,
              int my_rank, position my_pos)
{
    int dim_blk = n / lg_grid;
    int blk_size = dim_blk * dim_blk;

    if (my_rank == 0) {
        // place le bloc local de rank 0
        for (int i = 0; i < dim_blk; i++) {
            for (int j = 0; j < dim_blk; j++) {
                int gi = i;
                int gj = j;
                global_C[gi * n + gj] = local_C[i * dim_blk + j];
            }
        }

        // reçoit les autres blocs
        MPI_Status status;
        for (int r = 1; r < lg_grid * lg_grid; r++) {
            position p = get_position(r, lg_grid);
            int *tmp = malloc(blk_size * sizeof(int));
            if (!tmp) {
                fprintf(stderr, "Rank 0: erreur malloc tmp gather\n");
                MPI_Abort(MCW, 1);
            }
            MPI_Recv(tmp, blk_size, INT, r, 1, MCW, &status);

            for (int i = 0; i < dim_blk; i++) {
                for (int j = 0; j < dim_blk; j++) {
                    int gi = p.ligne * dim_blk + i;
                    int gj = p.colone * dim_blk + j;
                    global_C[gi * n + gj] = tmp[i * dim_blk + j];
                }
            }
            free(tmp);
        }
    } else {
        MPI_Send(local_C, blk_size, INT, 0, 1, MCW);
    }
}

// ==== main ====

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int my_rank, nb_proc;
    MPI_Comm_rank(MCW, &my_rank);
    MPI_Comm_size(MCW, &nb_proc);

    if (argc < 2) {
        if (my_rank == 0)
            fprintf(stderr, "Usage: %s N\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]); // taille globale de la matrice (n×n)

    int lg_grid = (int)(0.5 + sqrt(nb_proc)); // suppose nb_proc = lg_grid^2
    if (lg_grid * lg_grid != nb_proc) {
        if (my_rank == 0)
            fprintf(stderr, "Erreur: nb_proc doit être un carré parfait\n");
        MPI_Finalize();
        return 1;
    }

    int dim_blk = n / lg_grid;
    if (n % lg_grid != 0) {
        if (my_rank == 0)
            fprintf(stderr, "Erreur: N doit être divisible par sqrt(nb_proc)\n");
        MPI_Finalize();
        return 1;
    }

    position my_pos = get_position(my_rank, lg_grid);

    // Rank 0 lit les matrices A et B complètes
    int *A = NULL;
    int *B = NULL;
    int *C = NULL;

    if (my_rank == 0) {
        // ici tu peux remplacer par lecture("A.mat") / lecture("B.mat") si tu veux
        A = malloc(n * n * sizeof(int));
        B = malloc(n * n * sizeof(int));
        C = malloc(n * n * sizeof(int));

        if (!A || !B || !C) {
            fprintf(stderr, "Rank 0: erreur malloc A/B/C\n");
            MPI_Abort(MCW, 1);
        }

        // init simple pour test : A(i,j) = i+j, B(i,j) = i==j
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A[i * n + j] = i *n+ j;
                B[i * n + j] = (i == j) ? 1 : 0;
                C[i * n + j] = 0;
            }
        }
    }

    // Allocation des blocs locaux
    int blk_size = dim_blk * dim_blk;
    int *local_A = malloc(blk_size * sizeof(int));
    int *local_B = malloc(blk_size * sizeof(int));
    int *local_C = malloc(blk_size * sizeof(int));

    if (!local_A || !local_B || !local_C) {
        fprintf(stderr, "Rank %d: erreur malloc local\n", my_rank);
        MPI_Abort(MCW, 1);
    }

    // Distribution des blocs A et B
    // Si tu as déjà une fonction blocking() qui fait ça, utilise-la à la place
    if (my_rank == 0) {
        // envoie les blocs aux autres
        for (int r = 0; r < nb_proc; r++) {
            position p = get_position(r, lg_grid);
            int *blkA = malloc(blk_size * sizeof(int));
            int *blkB = malloc(blk_size * sizeof(int));

            for (int i = 0; i < dim_blk; i++) {
                for (int j = 0; j < dim_blk; j++) {
                    int gi = p.ligne * dim_blk + i;
                    int gj = p.colone * dim_blk + j;
                    blkA[i * dim_blk + j] = A[gi * n + gj];
                    blkB[i * dim_blk + j] = B[gi * n + gj];
                }
            }

            if (r == 0) {
                memcpy(local_A, blkA, blk_size * sizeof(int));
                memcpy(local_B, blkB, blk_size * sizeof(int));
            } else {
                MPI_Send(blkA, blk_size, INT, r, 10, MCW);
                MPI_Send(blkB, blk_size, INT, r, 11, MCW);
            }

            free(blkA);
            free(blkB);
        }
    } else {
        MPI_Status status;
        MPI_Recv(local_A, blk_size, INT, 0, 10, MCW, &status);
        MPI_Recv(local_B, blk_size, INT, 0, 11, MCW, &status);
    }

    // MPI_Barrier(MCW);
    // double t0 = MPI_Wtime();

    // Fox
    fox(n, lg_grid, my_rank, my_pos, local_A, local_B, local_C);

    // MPI_Barrier(MCW);
    // double t1 = MPI_Wtime();

    // if (my_rank == 0) {
    //     printf("Temps Fox MPI : %f s\n", t1 - t0);
    // }

    // Récupération de C globale
    if (my_rank == 0 && !C) {
        C = malloc(n * n * sizeof(int));
        if (!C) {
            fprintf(stderr, "Rank 0: erreur malloc C\n");
            MPI_Abort(MCW, 1);
        }
    }

    gather_C(C, local_C, n, lg_grid, my_rank, my_pos);

    if (my_rank == 0) {
        // pour debug : aff_mat(C, n);
        ecriture_txt(C, n, n, "C_fox.mat");
    }

    free(local_A);
    free(local_B);
    free(local_C);

    if (my_rank == 0) {
        free(A);
        free(B);
        free(C);
    }

    MPI_Finalize();
    return 0;
}
