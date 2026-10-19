#include <stdio.h>
#include <stdlib.h>

#include "../headers/gest_arg_init.h"

argument * init_arg(int argc, char ** argv){
    argument * ARG = malloc(sizeof(argument));

    if(argc ==1){
        ARG->nb_it = 20;
        ARG->taille = 50;
        ARG->type = 1;
        ARG->size = 8;
        fprintf(stderr,"execution par default : size %d, it %d, \n init grille de type centre avec taille 8\n",ARG->taille,ARG->nb_it);
    }
    if(argc == 2){
        ARG->taille = atoi(argv[1]);
        ARG->nb_it = 50;
        ARG->type = 1;
        ARG->size = 8;
    }
    if(argc == 3){
        ARG->taille = atoi(argv[1]);
        ARG->nb_it = atoi(argv[2]);
        ARG->type = 1;
        ARG->size = 8;
    }if(argc == 4){
        ARG->taille = atoi(argv[1]);
        ARG->nb_it = atoi(argv[2]);
        ARG->type = atoi(argv[3]);
        ARG->size = 8;
    }if(argc == 5){
        ARG->taille = atoi(argv[1]);
        ARG->nb_it = atoi(argv[2]);
        ARG->type = atoi(argv[3]);
        ARG->size = atoi(argv[4]);
    }
    return ARG;
}



