#include "../headers/gest_arg.h"

arg * parse_arg(int argc, char ** argv){
    arg * ARG = malloc(sizeof(arg));
    switch (argc)
    {
    case 2:
        ARG->nb_it = atoi(argv[1]);
        ARG->size_grille = 100;
        ARG->nb_individuis = 200;
        ARG->nb_infect = 20;
        break;
    case 3:
        ARG->nb_it = atoi(argv[1]);
        ARG->size_grille = atoi(argv[2]);        
        ARG->nb_individuis = 200;
        ARG->nb_infect = 20;

        break;
    case 4:
        ARG->nb_it = atoi(argv[1]);
        ARG->size_grille = atoi(argv[2]);
        ARG->nb_individuis = atoi(argv[3]);
        ARG->nb_infect = 20;
        break;
    case 5:
        ARG->nb_it = atoi(argv[1]);
        ARG->size_grille = atoi(argv[2]);
        ARG->nb_individuis = atoi(argv[3]);
        ARG->nb_infect = atoi(argv[4]);
        break;
    default:
        ARG->size_grille = 300;
        ARG->nb_individuis = 20000;
        ARG->nb_infect = 20;
        ARG->nb_it = 730;
        break;
    }
    if(ARG->nb_infect>ARG->nb_individuis){
        fprintf(stderr,"Erreur de nombre d'individus, Plus d'infecter que d'individus !\n");
    }
    return ARG;
}
