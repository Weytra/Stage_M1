#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../include/headers/gest_arg.h"
#include "../../include/headers/Structures.h"
#include "../../include/headers/Fonctions.h"
#include "../../include/headers/energy_monitor.h"


int main(int argc, char ** argv){
    int seed = 100;//seed pour le random
    arg * ARG = parse_arg(argc,argv);

    long long global_energy[2];
    long long algo_energy[2];
    double global_start_time = now();
    global_energy[0] = read_energy_uj();


    srand(seed);//on fixe la graine pour retrouver la meme situation initiale
    // simu final : 300x300, 20 000 individus et 20 infectés
    grid * G = make_grid(ARG->size_grille,ARG->nb_individuis,ARG->nb_infect);
    //affichage_grid(G);
    int ** analyse_tab = make_int_table(4,ARG->nb_it);


    analisys(G,analyse_tab,0);//analyse de la grille initiale

    double algo_start_time = now();
    algo_energy[0] = read_energy_uj();
    for(int i = 1;i<ARG->nb_it;i++){
        next_step(G);
        //
        // if(i%100 == 0)
        //     affichage_grid(G);//affichage de certaine iterations seulement

    }
    //aff_analisys(analyse_tab,ARG->nb_it);

double algo_end_time = now();

    algo_energy[1] = read_energy_uj();
    sup_grid(G);
    clock_t fin = clock();



    free_table(analyse_tab,ARG->nb_it);
    free(ARG);

    double global_end_time = now();
    global_energy[1] = read_energy_uj();

    {// benchmark
    BenchResult R_global;
    benchmark_end(&R_global,global_energy[0],global_start_time,global_energy[1],global_end_time);

    BenchResult R_algo;
    benchmark_end(&R_algo,algo_energy[0],algo_start_time,algo_energy[1],algo_end_time);

    //FullBenchResult F_b_r = {R_global,R_algo};
    printf("GLOBAL_TIME=%lf\n",
        R_global.time_s);

    printf("GLOBAL_ENERGY=%.12lf\n",
        R_global.energy_j);

    printf("GLOBAL_POWER=%lf\n",
        R_global.avg_power_w);

    printf("ALGO_TIME=%lf\n",
        R_algo.time_s);

    printf("ALGO_ENERGY=%.12lf\n",
        R_algo.energy_j);

    printf("ALGO_POWER=%lf\n",
        R_algo.avg_power_w);
    }



    return 0;
}


//tester avec la simu normale, avec 1000 iteration : 
//  je remarque qu deux individus sont bloquer en etat R sur au moins les 200 dernieres iteration