#ifndef MPI_GRID_H
#define MPI_GRID_H

#include <mpi.h>
#include "Structures.h"
#include "Fonctions.h"

// Grille locale pour un processus MPI (découpage par bandes horizontales)
typedef struct {
    cellule **act;        // grille locale active (ny_local x nx)
    cellule **next;       // grille locale suivante (ny_local x nx)
    int nx;               // largeur globale (nombre de colonnes)
    int ny_local;         // hauteur locale (lignes gérées par ce rang)
    int y_offset;         // indice de la première ligne globale gérée par ce rang
    int temps;            // temps (itération)
} grid_local;

// Bord résumé par colonne (pour l’infection aux frontières)
typedef struct {
    int nb_S;
    int nb_E;
    int nb_I;
    int nb_R;
} border_cell;


typedef struct {
    individus *data;   // tableau contigu d'individus
    int count;         // nombre d'individus
    int capacity;      // capacité allouée
} indiv_buffer;


// Création / destruction de la grille locale
grid_local *make_local_grid(int nx, int ny_local, int y_offset);
void free_local_grid(grid_local *G);

// Initialisation de la population locale
void init_local_population(grid_local *G,
                           int nb_indiv_local,
                           int nb_infect_local);

// Échange des bords entre voisins (haut/bas)
void exchange_borders(grid_local *G, MPI_Comm comm,
                      border_cell *top_border,
                      border_cell *bottom_border);

// Une étape locale (déplacement + infection + évolution)
void next_step_local(grid_local *G,
                     border_cell *top_border,
                     border_cell *bottom_border);


// Gestion des buffers
void buffer_init(indiv_buffer *buf);
void buffer_push(indiv_buffer *buf, individus *ind);

// Envoi / réception MPI
void send_buffer(indiv_buffer *buf, int dest, int tag, MPI_Comm comm);
void recv_buffer(indiv_buffer *buf, int src, int tag, MPI_Comm comm);

// Insertion des individus reçus
void insert_received_individuals(grid_local *G, indiv_buffer *buf, int row);

// Nouvelle version de next_step_local avec migration
void next_step_local_mg(grid_local *G,
                     border_cell *top_border,
                     border_cell *bottom_border,
                     MPI_Comm comm);

#endif
