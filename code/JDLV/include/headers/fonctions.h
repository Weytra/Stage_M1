#ifndef __FONCTIONS__
#define __FONCTIONS__

unsigned char * alloc_matrice_lin(int height,int width);
void top_ligne_lin(unsigned char * sous_mat,unsigned char * res,unsigned char * lg_inf,int size);
void down_ligne_lin(unsigned char * sous_mat,unsigned char * res,unsigned char * lg_inf,int size,int com_size);
void affichage_lin(unsigned char * matrice, int size);
void affichage_litle_lin(unsigned char * matrice, int size,int largeur);
void init_mat_lin(unsigned char * matrice, int size,short type,short taille);
unsigned char * next_step_lin(unsigned char * matrice,int size);

void init_mat(unsigned char ** matrice, int size,short type,short taille);
void affichage(unsigned char ** matrice, int size);
unsigned char ** next_step(unsigned char ** matrice, int size);
void free_mat(unsigned char ** mat, int size);
unsigned char ** alloc_matrice(int size);
void next_step_local(unsigned char *local,unsigned char *local_next,int size,int local_h);




#endif