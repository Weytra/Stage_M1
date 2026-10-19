#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../headers/Structures.h"
#include "../headers/Fonctions.h"

//ici les fonctions de la V1
grid * make_grid(int size, int nb_indiv,int nb_infect){
    //creation de la grille
    grid *G = (grid*)malloc(sizeof(grid));
    G->grille = make_ceil_grid(size);
    G->next_grille = make_ceil_grid(size);

    //initialisation des individus dans la grille
    int a,b,pos;
    for(int i = 0;i<nb_infect;i++){//genration des individus infecter
        a = rand_int(0,size-1);
        b = rand_int(0,size-1);
        if(G->grille[a][b].nb_indiv == 0){
            G->grille[a][b].indiv = (individus**)malloc(sizeof(individus*));
            G->grille[a][b].nb_indiv++;

            G->grille[a][b].indiv[0] = (individus *)malloc(sizeof(individus));

            G->grille[a][b].indiv[0]->status = I;
            G->grille[a][b].indiv[0]->dE = new_v_dE();
            G->grille[a][b].indiv[0]->dI = new_v_dI();
            G->grille[a][b].indiv[0]->dR = new_v_dR();
            G->grille[a][b].indiv[0]->tps_in_status = 0;;

        }
        else{
            G->grille[a][b].indiv = realloc(G->grille[a][b].indiv,(G->grille[a][b].nb_indiv+1)*sizeof(individus*));
            
            pos = G->grille[a][b].nb_indiv;
            G->grille[a][b].nb_indiv++;

            G->grille[a][b].indiv[pos] = (individus *)malloc(sizeof(individus));
            G->grille[a][b].indiv[pos]->status = I;
            G->grille[a][b].indiv[pos]->dE = new_v_dE();
            G->grille[a][b].indiv[pos]->dI = new_v_dI();
            G->grille[a][b].indiv[pos]->dR = new_v_dR();
            G->grille[a][b].indiv[pos]->tps_in_status = 0;;
        }
    }
    // printf("jusque iiiiiiiiiiiiiiiici cest bon\n");

    for(int i = 0;i<nb_indiv-nb_infect;i++){//generations des autres individus
        
        a = rand_int(0,size-1);
        b = rand_int(0,size-1);
        // printf("jusque ici cest bon : i = %d a = %d b = %d\n",i,a,b);

        if(G->grille[a][b].nb_indiv == 0){
            // printf("dans le if : a = %d, b= %d\n",a,b);
            

            G->grille[a][b].indiv = (individus**)malloc(sizeof(individus*));
            G->grille[a][b].nb_indiv++;
            // printf("dans le if 2\n");
            G->grille[a][b].indiv[0] = (individus *)malloc(sizeof(individus));


            G->grille[a][b].indiv[0]->status = S;
            G->grille[a][b].indiv[0]->dE = new_v_dE();
            G->grille[a][b].indiv[0]->dI = new_v_dI();
            G->grille[a][b].indiv[0]->dR = new_v_dR();
            G->grille[a][b].indiv[0]->tps_in_status = 0;
            // printf("dans le if \n");

        }
        else{
            // printf("dans le elseeee\n");

            G->grille[a][b].indiv = realloc(G->grille[a][b].indiv,(G->grille[a][b].nb_indiv+1)*sizeof(individus*));
            pos = G->grille[a][b].nb_indiv;
            G->grille[a][b].indiv[pos] = (individus *)malloc(sizeof(individus));

            G->grille[a][b].nb_indiv++;

            
            G->grille[a][b].indiv[pos]->status = S;
            G->grille[a][b].indiv[pos]->dE = new_v_dE();
            G->grille[a][b].indiv[pos]->dI = new_v_dI();
            G->grille[a][b].indiv[pos]->dR = new_v_dR();
            G->grille[a][b].indiv[pos]->tps_in_status = 0;
            
        }

        //aff_indiv(G->grille[a][b].indiv[0]);

    }

    G->size = size;
    G->nb_indiv = nb_indiv;
    G->indiv_inf = nb_infect;

    return G;
}

void aff_indiv(individus * indiv){
    printf("etats = ");
    switch (indiv->status)
    {
    case N :
        printf("N");
        break;

    case E :
        printf("E");
        break;

    case I :
        printf("I");
        break;

    case R :
        printf("R");
        break;    
    default:
        break;
    }
    printf(":\n         de %d, di %d, dr %d\n         tps : %f\n",indiv->dE,indiv->dI,indiv->dR,indiv->tps_in_status);

}

void affichage_grid(grid * G){
    //dans cette fonciton on affiche que le premier individus present sur une case.
    for(int i = 0;i<G->size;i++){
        printf("# %d |",i);
        for(int j = 0;j<G->size;j++){
            if (G->grille[i][j].nb_indiv == 0){
                printf(".");
            }else if(G->grille[i][j].indiv[0]->status == S)
                printf("S");
            else if(G->grille[i][j].indiv[0]->status == E)
                printf("E");
            else if(G->grille[i][j].indiv[0]->status == I)
                printf("I");
            else if(G->grille[i][j].indiv[0]->status == R)
                printf("R");
            else{
                fprintf(stderr,"error inprobable value in this case : grille[%d,%d] = %d",i,j,G->grille[i][j].indiv[0]->status);
            }
        }
        printf("\n");
    }
    printf("\n\n");
}

void sup_grid(grid *G) {
    free_ceil_grid_2(G->grille,G->size);
    free_ceil_grid(G->next_grille,G->size);
    //free(G->next_grille);
    free(G);
}

void aff_case(cellule elt){
    printf("nb elt in this case : %d:\n",elt.nb_indiv);
    for(int i = 0;i<elt.nb_indiv;i++){
        aff_indiv(elt.indiv[i]);
    }
}

void next_step(grid *G) {
    int size = G->size;
    cellule **act  = G->grille;   // grille de lecture
    cellule **next = G->next_grille;  // grille d’écriture

    // 1) Vider la grille next (sans free)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            next[i][j].nb_indiv = 0;
            // On ne free pas next[i][j].indiv : on réutilise le buffer existant
        }
    }

    // 2) Déplacement local des individus
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            if (act[i][j].nb_indiv == 0)
                continue;

            for (int k = 0; k < act[i][j].nb_indiv; k++) {

                individus *ind = act[i][j].indiv[k];

                // Déplacement local dans les 9 cases voisines
                int dx = rand_int(-1, 1);
                int dy = rand_int(-1, 1);

                int ni = (i + dx + size) % size;
                int nj = (j + dy + size) % size;

                // Ajout dans next[ni][nj]
                int pos = next[ni][nj].nb_indiv;

                if (pos == 0) {
                    // Première insertion → allocation si nécessaire
                    if (next[ni][nj].indiv == NULL)
                        next[ni][nj].indiv = malloc(sizeof(individus*));
                } else {
                    // Agrandir le tableau de pointeurs
                    next[ni][nj].indiv = realloc(
                        next[ni][nj].indiv,
                        (pos + 1) * sizeof(individus*)
                    );
                }

                next[ni][nj].indiv[pos] = ind;
                next[ni][nj].nb_indiv++;
            }
        }
    }

    // 3) Calcul infection + évolution sur la grille next
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            if (next[i][j].nb_indiv == 0)
                continue;

            int infect_voisin = 0;
            int suseptible = 0;

            // Comptage dans la cellule
            for (int k = 0; k < next[i][j].nb_indiv; k++) {
                if (next[i][j].indiv[k]->status == S) suseptible++;
                if (next[i][j].indiv[k]->status == I) infect_voisin++;
            }

            // Comptage dans les 8 voisins
            if (suseptible > 0) {
                for (int p = -1; p <= 1; p++) {
                    for (int q = -1; q <= 1; q++) {
                        int ni = (i + p + size) % size;
                        int nj = (j + q + size) % size;

                        for (int r = 0; r < next[ni][nj].nb_indiv; r++) {
                            if (next[ni][nj].indiv[r]->status == I)
                                infect_voisin++;
                        }
                    }
                }

                // Infection
                for (int k = 0; k < next[i][j].nb_indiv; k++) {
                    individus *ind = next[i][j].indiv[k];
                    if (ind->status == S && infection(infect_voisin)) {
                        ind->status = E;
                        ind->tps_in_status = 0;
                    }
                }
            }

            // Évolution E → I → R → S
            for (int k = 0; k < next[i][j].nb_indiv; k++) {
                individus *ind = next[i][j].indiv[k];

                switch (ind->status) {
                    case E:
                        if (ind->tps_in_status >= ind->dE) {
                            ind->status = I;
                            ind->tps_in_status = 0;
                        } else ind->tps_in_status++;
                        break;

                    case I:
                        if (ind->tps_in_status >= ind->dI) {
                            ind->status = R;
                            ind->tps_in_status = 0;
                        } else ind->tps_in_status++;
                        break;

                    case R:
                        if (ind->tps_in_status >= ind->dR) {
                            ind->status = S;
                            ind->tps_in_status = 0;
                        } else ind->tps_in_status++;
                        break;

                    default:
                        break;
                }
            }
        }
    }

    // 4) Swap des grilles
    cellule **tmp = G->grille;
    G->grille = G->next_grille;
    G->next_grille = tmp;

    G->temps++;
}

cellule ** make_ceil_grid(int size){
    cellule ** grille = (cellule **)malloc(size*sizeof(cellule*));
    for(int i = 0;i<size;i++){
        grille[i] = (cellule*)malloc(size * sizeof(cellule));
        for(int j = 0;j<size;j++){
            grille[i][j].indiv = NULL;
            grille[i][j].nb_indiv = 0;
        }
    }
    return grille;
}

void free_ceil_grid(cellule ** tab,int size){
    for(int i = 0;i<size;i++){
        for(int j = 0;j<size;j++){
            free(tab[i][j].indiv);
        }
        free(tab[i]);
    }
    free(tab);
}

void free_ceil_grid_2(cellule **tab, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            cellule *c = &tab[i][j];

            // Libérer chaque individus* si présent
            for (int k = 0; k < c->nb_indiv; k++) {
                free(c->indiv[k]);
            }
            // Libérer le tableau de pointeurs
            free(c->indiv);
            //printf("oeoeoeoe\n");
        }
        free(tab[i]);   // ligne
    }
    free(tab);          // grille
}


double rand_f(int rd_max){ return rand() / (double)rd_max;} 

float negExp(double inMean) {
    return -inMean * log(1 - rand_f(RAND_MAX));
}
//Pour eviter les erreurs de frappe
float new_v_dE(){
    return negExp(3);
}
float new_v_dI(){
    return negExp(7);
}
float new_v_dR(){
    return negExp(365);
}

int rand_int(int a, int b) {
    return a + rand() % (b - a + 1);
}

short infection(int nb_voisin){
    float p = 1 - exp(-0.5 * nb_voisin);
    double x = rand() / (double)RAND_MAX;
    if (x<p)
        return 1;
    return 0;
}

int ** make_int_table(int x,int y){//y = hauteur, x largeur
    int ** tab = (int **)malloc(y*sizeof(int*));
    for(int i = 0;i<y;i++){
        tab[i] = (int *)malloc(x*sizeof(int));
        for(int j = 0;j<x;j++){
            tab[i][j] = 0;
        }
    }
    return tab;
}

void analisys(grid * G,int ** stats, int it){
    for(int i = 0;i<G->size;i++){
        for(int j = 0;j<G->size;j++){
            if(G->grille[i][j].nb_indiv){
                for(int k = 0;k<G->grille[i][j].nb_indiv;k++){
                    switch (G->grille[i][j].indiv[k]->status)
                    {
                    case S :
                        stats[it][0]++;
                        break;
                    case E :
                        stats[it][1]++;
                        break;
                    case I :
                        stats[it][2]++;
                        break;
                    case R:
                        stats[it][3]++;
                        break;
                    
                    default:
                        break;
                    }
                }
            }
        }
    }
}

void aff_analisys(int **tab,int nb_it){
    for(int i = 0;i<nb_it;i++){
        printf("itération %d : \n   S : %d\n    E : %d\n    I : %d\n    R : %d\n\n",i,
            tab[i][0],tab[i][1],tab[i][2],tab[i][3]);  
    }
}

void free_table(int ** tab,int y){
    for(int i = 0;i<y;i++){
        free(tab[i]);
    }
    free(tab);
}