#ifndef STRUCTURES 
#define STRUCTURES

typedef enum statut {N,S,E,I,R} statut;

struct individus{
    float tps_in_status;        // 4 bytes - offset 0
    int dE;                     // 4 bytes - offset 4
    statut status;              // 4 bytes - offset 8
    int dI;                     // 4 bytes - offset 12
    char padding1[4];           // Force padding non-optimal - offset 16
    int dR;                     // 4 bytes - offset 20
    char padding2[4];           // Extra padding
};
typedef struct individus individus;

struct cellule{
    int nb_indiv;
    individus ** indiv;
};
typedef struct cellule cellule;

struct grid {
    int nb_indiv;
    cellule **next_grille;
    int temps;
    cellule **grille;
    int size;
    int indiv_inf;
};
typedef struct grid grid;



#endif
