#ifndef STRUCTURES_CACHE_FRIENDLY
#define STRUCTURES_CACHE_FRIENDLY

typedef enum statut {N,S,E,I,R} statut;

struct individus{
    statut status;              // 4 bytes - offset 0 (enum)
    int dE;                     // 4 bytes - offset 4
    int dI;                     // 4 bytes - offset 8
    int dR;                     // 4 bytes - offset 12
    float tps_in_status;        // 4 bytes - offset 16
};
typedef struct individus individus;

struct cellule{
    int nb_indiv;
    individus ** indiv;
};
typedef struct cellule cellule;

struct grid {
    int size;
    int temps;
    int nb_indiv;
    int indiv_inf;
    cellule **grille;
    cellule **next_grille;
};
typedef struct grid grid;

#endif
