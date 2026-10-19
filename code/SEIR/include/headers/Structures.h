#ifndef STRUCTURES 
#define STRUCTURES

typedef enum statut {N,S,E,I,R} statut;//None (ie pas d'individus) Susceptible, Exposed, Infected et Recovered 

struct individus{
    statut status ;//etat actuel de l'individus
    float tps_in_status;//temps passé dans cette etat
    int dE,dI,dR;//temps qu'il doit passer dans chaque etat
};
typedef struct individus individus;

struct cellule{
    int nb_indiv;
    individus ** indiv;
};
typedef struct cellule cellule;

struct grid {
    cellule **grille;
    cellule **next_grille;
    int temps;
    int size;
    int nb_indiv;
    int indiv_inf;

};
typedef struct grid grid;


#endif
