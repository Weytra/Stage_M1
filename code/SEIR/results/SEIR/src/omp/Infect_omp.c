#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#include "../../include/headers/gest_arg.h"
#include "../../include/headers/Structures.h"
#include "../../include/headers/Fonctions.h"
#include "../../include/headers/energy_monitor.h"


void next_step_omp(grid *G) {
    int size = G->size;
    cellule **act  = G->grille;
    cellule **next = G->next_grille;

    int T = omp_get_max_threads();
    // ================================
    // 1) ALLOCATION DES GRILLES LOCALES
    // ================================
    cellule ***next_local = malloc(T * sizeof(cellule**));

    for (int t = 0; t < T; t++) {
        next_local[t] = malloc(size * sizeof(cellule*));
        for (int i = 0; i < size; i++) {
            next_local[t][i] = malloc(size * sizeof(cellule));
            for (int j = 0; j < size; j++) {
                next_local[t][i][j].nb_indiv = 0;
                next_local[t][i][j].indiv = NULL;
            }
        }
    }

    // ================================
    // 2) DEPLACEMENT PARALLELE
    // ================================
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        cellule **local = next_local[tid];

        #pragma omp for collapse(2)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {

                if (act[i][j].nb_indiv == 0)
                    continue;

                for (int k = 0; k < act[i][j].nb_indiv; k++) {

                    individus *ind = act[i][j].indiv[k];

                    int dx = rand_int(-1, 1);
                    int dy = rand_int(-1, 1);

                    int ni = (i + dx + size) % size;
                    int nj = (j + dy + size) % size;

                    int pos = local[ni][nj].nb_indiv;

                    if (pos == 0) {
                        local[ni][nj].indiv = malloc(sizeof(individus*));
                    } else {
                        local[ni][nj].indiv = realloc(
                            local[ni][nj].indiv,
                            (pos + 1) * sizeof(individus*)
                        );
                    }

                    local[ni][nj].indiv[pos] = ind;
                    local[ni][nj].nb_indiv++;
                }
            }
        }
    }

    // ================================
    // 3) FUSION DES GRILLES LOCALES
    // ================================
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            next[i][j].nb_indiv = 0;
            next[i][j].indiv = NULL;

            for (int t = 0; t < T; t++) {
                int nloc = next_local[t][i][j].nb_indiv;
                if (nloc == 0) continue;

                int old = next[i][j].nb_indiv;

                if (old == 0) {
                    next[i][j].indiv = malloc(nloc * sizeof(individus*));
                } else {
                    next[i][j].indiv = realloc(
                        next[i][j].indiv,
                        (old + nloc) * sizeof(individus*)
                    );
                }

                memcpy(
                    next[i][j].indiv + old,
                    next_local[t][i][j].indiv,
                    nloc * sizeof(individus*)
                );

                next[i][j].nb_indiv += nloc;
            }
        }
    }

    // ================================
    // 4) LIBERATION DES GRILLES LOCALES
    // ================================
    for (int t = 0; t < T; t++) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                free(next_local[t][i][j].indiv);
            }
            free(next_local[t][i]);
        }
        free(next_local[t]);
    }
    free(next_local);

    // ================================
    // 5) INFECTION + EVOLUTION (PARALLELE)
    // ================================
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            if (next[i][j].nb_indiv == 0)
                continue;

            int infect_voisin = 0;
            int suseptible = 0;

            for (int k = 0; k < next[i][j].nb_indiv; k++) {
                if (next[i][j].indiv[k]->status == S) suseptible++;
                if (next[i][j].indiv[k]->status == I) infect_voisin++;
            }

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

                for (int k = 0; k < next[i][j].nb_indiv; k++) {
                    individus *ind = next[i][j].indiv[k];
                    if (ind->status == S && infection(infect_voisin)) {
                        ind->status = E;
                        ind->tps_in_status = 0;
                    }
                }
            }

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
                }
            }
        }
    }

    // ================================
    // 6) SWAP DES GRILLES
    // ================================
    cellule **tmp = G->grille;
    G->grille = G->next_grille;
    G->next_grille = tmp;

    G->temps++;
}



void next_step_omp_2(grid *G) {
    int size = G->size;
    cellule **act  = G->grille;   // grille de lecture
    cellule **next = G->next_grille;  // grille d’écriture

    // 1) Vider la grille next (sans free)
    #pragma omp parallel for collapse(2)
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
    #pragma omp parallel for collapse(2)
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

   

int main(int argc, char ** argv){

    int seed = 100;//seed pour le random
    arg * ARG = parse_arg(argc,argv);

    int T = omp_get_max_threads();


    long long global_energy[2];
    long long algo_energy[2];
    double global_start_time = now();
    global_energy[0] = read_energy_uj();


    srand(seed);//on fixe la graine pour retrouver la meme situation initiale
    // simu final : 300x300, 20 000 individus et 20 infectés
    grid * G = make_grid(ARG->size_grille,ARG->nb_individuis,ARG->nb_infect);
    //affichage_grid(G);
    int ** analyse_tab = make_int_table(4,ARG->nb_it);
    fprintf(stderr,"nb it = %d,size grille : %d,  nb indiv %d, nb infect %d\n",ARG->nb_it,ARG->size_grille,ARG->nb_individuis,ARG->nb_infect);

    analisys(G,analyse_tab,0);//analyse de la grille initiale

    double algo_start_time = now();
    algo_energy[0] = read_energy_uj();

    fprintf(stderr,"on a %d thread\n",T);


    for(int i = 1;i<ARG->nb_it;i++){
        next_step_omp(G);
        //analisys(G,analyse_tab,i);
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
