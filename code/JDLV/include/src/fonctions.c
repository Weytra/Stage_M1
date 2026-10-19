#include <stdio.h>
#include <stdlib.h>

#include "../headers/fonctions.h"

void init_mat(unsigned char **matrice, int size, short type, short taille)
{
    // type 1 : carré centré
    if(type == 1){
        int start = size/2 - taille/2;
        int end   = start + taille;

        for(int i = start; i < end; i++){
            for(int j = start; j < end; j++){
                matrice[i][j] = 1;
            }
        }
    }
    else if(type == 0){
        // type 0 : damier ou motif périodique
        for(int i = 1; i < size-1; i++){
            for(int j = 1; j < size-1; j++){
                matrice[i][j] = ((i + j) & 1) == 0;  // damier
            }
        }
    }
    else if(type == 2){
        for(int i = 1; i < size-1; i++){
            for(int j = 1; j < size-1; j++){
                matrice[i][j] = (rand() % 100 < 40);   // 40% de vivantes
            }
        }
    }
    else(fprintf(stderr,"erreur 2\n"));
}


unsigned char ** alloc_matrice(int size){
    unsigned char ** matrice = (unsigned char**)malloc(size*sizeof(unsigned char*));
    for(int i = 0;i<size;i++){
        matrice[i] = (unsigned char * )malloc(size * sizeof(unsigned char));
        for(int j = 0;j<size;j++){
            matrice[i][j] = 0; //initialisation vide
        }
    }
    return matrice;
}

void free_mat(unsigned char ** mat, int size){
    for(int i = 0;i<size;i++){
        free(mat[i]);
    }
    free(mat);
}

unsigned char ** next_step(unsigned char ** matrice, int size){
    unsigned char **res = alloc_matrice(size);
    printf("on est la \n");
    for(int i = 1; i < size-1; i++){
        for(int j = 1; j < size-1; j++){

            int cpt = 0;

            // Comptage des voisins
            for(int di = -1; di <= 1; di++){
                for(int dj = -1; dj <= 1; dj++){
                    if(di == 0 && dj == 0) continue; // on ignore la cellule elle-même
                    cpt += matrice[i+di][j+dj];
                }
            }

            // Application des règles
            if(matrice[i][j] == 1){
                // cellule vivante
                res[i][j] = (cpt == 2 || cpt == 3);
            } else {
                // cellule morte
                res[i][j] = (cpt == 3);
            }
        }
    }
    free_mat(matrice,size);

    return res;
}

void affichage(unsigned char ** matrice, int size){
    for(int i = 0;i< size;i++){
        for(int j = 0; j<size;j++){
            if (matrice[i][j] == 0)
                printf(" ");
            else{
                if(matrice[i][j] == 1)
                    printf("+");
                else{
                    fprintf(stderr,"Erreur de valeure dans la matrice\n");
                    printf("valeur : %c\n",matrice[i][j]);
                }
            }
        }
        printf("\n");
    }
    printf("\n\n");
}



void init_mat_lin(unsigned char *matrice, int size, short type, short taille)
{
    // type 1 : carré centré
    if(type == 1){
        int start = size/2 - taille/2;
        int end   = start + taille;

        for(int i = start; i < end; i++){
            for(int j = start; j < end; j++){
                matrice[i*size + j] = 1;
            }
        }
        return;
    }
    else 
    // type 0 : damier
    if(type == 0){
        for(int i = 1; i < size-1; i++){
            for(int j = 1; j < size-1; j++){
                matrice[i*size + j] = ((i + j) & 1) == 0;
            }
        }
        return;
    }
    else 
    // type 2 : aléatoire
    if(type == 2){
        for(int i = 1; i < size-1; i++){
            for(int j = 1; j < size-1; j++){
                matrice[i* size +j] = (rand() % 100 < 40);   // 40% de vivantes
            }
        }
        return;
    }
    else 
        fprintf(stderr, "Erreur : type d'initialisation inconnu (%d)\n", type);

}

unsigned char * alloc_matrice_lin(int height,int width){
    unsigned char * matrice = (unsigned char*)malloc(height*width*sizeof(unsigned char));
    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            matrice[i*width+j] = 0; //initialisation vide
        }
    }
    return matrice;
}

void top_ligne_lin(unsigned char * sous_mat,unsigned char * res,unsigned char * lg_inf,int size){
    int cpt;
    int i = 0;
    for(int j = 1; j<size-1;j++){
        cpt = 0;
        //il faut donc pour compter les voisin compter les sur les deux ligne de sous mat et sur la ligne lg_inf
        for(int k = 0; k<2;k++){//prend donc les deux premire lignes de sous mat 
            for(int m = -1; m<2;m++){
                if(sous_mat[(i+k)*size + j+m] == 1){
                    cpt++;
                }
            }
        }
        //calcul de la ligne lg_inf
        for(int p = -1;p<2;p++){
            if(lg_inf[j+p] == 1){
                cpt++;
            }
        }
        cpt -= sous_mat[i*size+j]; 

        if(cpt > 3)
            res[i*size +j] = 0;
        else{
            if(cpt == 3)
                res[i*size+j ] = 1;
            else{
                if(cpt < 2) 
                    res[i*size+j] = 0;
                else
                    res[i*size+j] = sous_mat[i*size+j];
            }
        }  
    }
}

void down_ligne_lin(unsigned char * sous_mat,unsigned char * res,unsigned char * lg_inf,int size,int com_size){
    int cpt;
    int i = size/com_size;
    for(int j = 1; j<size-1;j++){
        cpt = 0;
        //il faut donc pour compter les voisin compter les sur les deux ligne de sous mat et sur la ligne lg_inf
        for(int k = -1; k<1;k++){//prend donc les deux derniere lignes de sous mat 
            for(int m = -1; m<2;m++){
                if(sous_mat[(i+k)*size + j+m] == 1){
                    cpt++;
                }
            }
        }
        //calcul de la ligne lg_inf
        for(int p = -1;p<2;p++){
            if(lg_inf[j+p] == 1){
                cpt++;
            }
        }
        cpt -= sous_mat[i*size+j]; 

        if(cpt > 3)
            res[i*size +j] = 0;
        else{
            if(cpt == 3)
                res[i*size+j ] = 1;
            else{
                if(cpt < 2) 
                    res[i*size+j] = 0;
                else
                    res[i*size+j] = sous_mat[i*size+j];
            }
        }  
    }
}


void affichage_lin(unsigned char * matrice, int size){
    for(int i = 0;i< size;i++){
        for(int j = 0; j<size;j++){
            if (matrice[i*size+j] == 0)
                printf(" ");
            else{
                if(matrice[i*size+j] == 1)
                    printf("#");
                else{
                    fprintf(stderr,"Erreur de valeure dans la matrice\n");
                    printf("valeur : %c\n",matrice[i*size+j]);
                }
            }
        }
        printf("\n");
    }
}

void affichage_litle_lin(unsigned char * matrice, int size,int largeur){
    printf("\n");
    for(int i = 0;i< largeur;i++){
        for(int j = 0; j<size;j++){
            if (matrice[i*size+j] == 0)
                printf(".");
            else{
                if(matrice[i*size+j] == 1)
                    printf("#");
                else{
                    fprintf(stderr,"Erreur de valeure dans la matrice\n");
                    printf("valeur : %02X\n",matrice[i*size+j]);
                }
            }
        }
        printf("\n");
    }
    printf("\n\n");
}


unsigned char * next_step_lin(unsigned char * matrice,int size){
    unsigned char *res = alloc_matrice_lin(size,size);
    int cpt;
    for(int i = 1;i< size -1;i++){
        for(int j = 1; j<size-1;j++){
            cpt = 0;
            for(int k = -1; k<2;k++){
                for(int m = -1; m<2;m++){
                    if(matrice[(i+k)*size + j+m] == 1){
                        cpt++;
                    }
                }
            }
            
            if(matrice[i*size +j])
                cpt--;//on enleve la valeur si la celule tester est vivante
            //printf("cpt = %d\n",cpt);
            if(cpt > 3)
                res[i*size +j] = 0;
            else{
                if(cpt == 3)
                    res[i*size+j] = 1;
                else{
                    if(cpt < 2)
                        res[i*size+j] = 0;
                    else
                        res[i*size+j] = matrice[i*size+j];
                }
            }
            
        }
    }
    free(matrice);
    return res;
}

// Calcul de la prochaine étape sur la portion locale (avec halos) MPI !
void next_step_local(unsigned char *local,
                     unsigned char *local_next,
                     int size,
                     int local_h)
{
    for (int i = 1; i <= local_h; i++) {
        for (int j = 1; j < size-1; j++) {
            int cpt = 0;

            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    cpt += local[(i+di)*size + (j+dj)];
                }
            }
            cpt -= local[i*size + j];

            if (cpt == 3)
                local_next[i*size + j] = 1;
            else if (cpt == 2)
                local_next[i*size + j] = local[i*size + j];
            else
                local_next[i*size + j] = 0;
        }
    }
}