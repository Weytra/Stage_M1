#ifndef __gest_arg__
#define __gest_arg__

typedef struct argument {
    int taille;
    int nb_it;
    short type;
    short size;
} argument;

argument * init_arg(int argc, char ** argv);


#endif