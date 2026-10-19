#ifndef FONCTIONS
#define FONCTIONS

#include "Structures.h"

//fonctions strucurelles    
grid * make_grid(int size, int nb_indiv,int nb_infect);
void sup_grid(grid * G);
void next_step(grid * G);
void next_step_omp(grid * G);
cellule ** make_ceil_grid(int size);
void free_ceil_grid(cellule ** grille,int size);
int ** make_int_table(int x,int y);
void free_table(int ** tab,int x);
void free_ceil_grid_2(cellule **tab, int size);


//fonctions d'affichage
void affichage_grid(grid * G);
void aff_indiv(individus * indiv);
void aff_case(cellule elt);
void aff_analisys(int **tab,int nb_it);

//fonctions de generation
int rand_int(int a, int b);
float new_v_dE();
float new_v_dI();
float new_v_dR();
float negExp(double inMean);
double rand_f(int rd_max);
short infection(int nb_voisin);

//fonction d'analyse
void analisys(grid * G,int **stats ,int it);

#endif