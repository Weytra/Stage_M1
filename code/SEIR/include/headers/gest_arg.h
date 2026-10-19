#ifndef __ARG__
#define __ARG__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct arg{
    int nb_it;
    int size_grille;
    int nb_individuis;
    int nb_infect;
}arg;




arg * parse_arg(int argc, char ** argv);


#endif