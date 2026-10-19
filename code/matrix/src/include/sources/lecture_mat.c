#include <stdio.h>
#include <stdlib.h>
#include "../headers/lecture_mat.h"

Matrix lecture(char *fichier)
{
    FILE *fic = fopen(fichier, "rb");

    if (fic == NULL) {
        perror("fopen");
        printf("fichier non trouve exit()\n");
        exit(0);
    }

    Matrix M;

    int a= fread(&M.rows, sizeof(int), 1, fic);
    a = fread(&M.cols, sizeof(int), 1, fic);

    M.data = malloc(M.rows * M.cols * sizeof(int));

    a = fread(M.data, sizeof(int), M.rows * M.cols, fic);

    fclose(fic);

    return M;
}

Matrix_f lecture_float(char *fichier)
{
    FILE *fic = fopen(fichier, "rb");

    if (fic == NULL) {
        perror("fopen");
        printf("fichier non trouve exit()\n");
        exit(0);
    }

    Matrix_f M;

    int a= fread(&M.rows, sizeof(int), 1, fic);
    a = fread(&M.cols, sizeof(int), 1, fic);

    M.data = malloc(M.rows * M.cols * sizeof(float));

    a = fread(M.data, sizeof(float), M.rows * M.cols, fic);

    fclose(fic);

    return M;
}

void ecriture(int *mat, int lg, int col, char *name){
    FILE *fic = fopen(name, "w");
    fwrite(&lg, sizeof(int), 1, fic);
    fwrite(&col, sizeof(int), 1, fic);
    fwrite(mat, sizeof(int), lg*col, fic);
    fclose(fic);
}

void ecriture_f(float *mat, int lg, int col, char *name){
    FILE *fic = fopen(name, "w");
    fwrite(&lg, sizeof(int), 1, fic);
    fwrite(&col, sizeof(int), 1, fic);
    fwrite(mat, sizeof(float), lg*col, fic);
    fclose(fic);
}


void ecriture_txt(int *mat, int lg, int col, char *fichier)
{
    FILE *fic = fopen(fichier, "w");
    if (!fic) {
        perror("fopen");
        return;
    }

    // écrire les dimensions
    fprintf(fic, "%d %d\n", lg, col);

    // écrire la matrice
    for (int i = 0; i < lg; i++) {
        for (int j = 0; j < col; j++) {
            fprintf(fic, "%d ", mat[i * col + j]);
        }
        fprintf(fic, "\n");
    }
    fclose(fic);
}




