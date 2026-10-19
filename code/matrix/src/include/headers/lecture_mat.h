#ifndef __lecture_H__
#define __lecture_H__

typedef struct {
    int *data;
    int rows;
    int cols;
} Matrix;

typedef struct {
    float *data;
    int rows;
    int cols;
} Matrix_f;

Matrix lecture(char *fichier);

Matrix_f lecture_float(char *fichier);

/* la fonction lit le fichier en .mat et retourne cette matrice sous
la forme d'une matrice 1D, deja aloué*/

void ecriture(int * mat, int lg,int col, char * name);
/* Prend une matrice 1D et l'ecrit dans un fichier .mat binaire*/
void ecriture_f(float *mat, int lg, int col, char *name);
void ecriture_txt(int *mat, int lg, int col, char *fichier);
/* Prend une matrice 1D et l'ecrit dans un fichier .txt lisibel*/


#endif