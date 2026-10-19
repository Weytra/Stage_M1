#include <stdio.h>
#include <stdlib.h>
#include "fonctions_mpi.h"

// ---------- Helpers internes pour la grille rectangulaire ----------

static cellule **alloc_rect_grid(int ny, int nx) {
    cellule **tab = malloc(ny * sizeof(cellule *));
    for (int i = 0; i < ny; i++) {
        tab[i] = malloc(nx * sizeof(cellule));
        for (int j = 0; j < nx; j++) {
            tab[i][j].indiv = NULL;
            tab[i][j].nb_indiv = 0;
        }
    }
    return tab;
}

static void free_rect_grid(cellule **tab, int ny, int nx) {
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            cellule *c = &tab[i][j];
            for (int k = 0; k < c->nb_indiv; k++) {
                free(c->indiv[k]);
            }
            free(c->indiv);
        }
        free(tab[i]);
    }
    free(tab);
}

// ---------- Création / destruction de la grille locale ----------

grid_local *make_local_grid(int nx, int ny_local, int y_offset) {
    grid_local *G = malloc(sizeof(grid_local));
    G->nx       = nx;
    G->ny_local = ny_local;
    G->y_offset = y_offset;
    G->temps    = 0;

    G->act  = alloc_rect_grid(ny_local, nx);
    G->next = alloc_rect_grid(ny_local, nx);

    return G;
}

void free_local_grid(grid_local *G) {
    if (!G) return;

    // Libérer les individus uniquement dans act
    free_rect_grid(G->act, G->ny_local, G->nx);

    // Dans next : libérer seulement les tableaux de pointeurs
    for (int i = 0; i < G->ny_local; i++) {
        for (int j = 0; j < G->nx; j++) {
            free(G->next[i][j].indiv);  // mais PAS les individus !
        }
        free(G->next[i]);
    }
    free(G->next);

    free(G);
}


// ---------- Initialisation de la population locale ----------

void init_local_population(grid_local *G,
                           int nb_indiv_local,
                           int nb_infect_local) {
    int nx = G->nx;
    int ny = G->ny_local;

    // Placement des individus
    for (int n = 0; n < nb_indiv_local; n++) {
        int x = rand_int(0, nx - 1);
        int y = rand_int(0, ny - 1);

        cellule *c = &G->act[y][x];
        int pos = c->nb_indiv;

        if (pos == 0) {
            c->indiv = malloc(sizeof(individus *));
        } else {
            c->indiv = realloc(c->indiv, (pos + 1) * sizeof(individus *));
        }

        c->indiv[pos] = malloc(sizeof(individus));
        individus *ind = c->indiv[pos];

        ind->status        = S;
        ind->tps_in_status = 0.0f;
        ind->dE = new_v_dE();
        ind->dI = new_v_dI();
        ind->dR = new_v_dR();

        c->nb_indiv++;
    }

    // Infecter quelques individus localement
    if (nb_infect_local > 0 && nb_indiv_local > 0) {
        int ecart = nb_indiv_local / nb_infect_local;
        if (ecart == 0) ecart = 1;

        int cpt = 0;
        for (int i = 0; i < ny; i++) {
            for (int j = 0; j < nx; j++) {
                cellule *c = &G->act[i][j];
                for (int k = 0; k < c->nb_indiv; k++) {
                    if (cpt % ecart == 0 && nb_infect_local > 0) {
                        c->indiv[k]->status = I;
                        nb_infect_local--;
                        if (nb_infect_local == 0) return;
                    }
                    cpt++;
                }
            }
        }
    }
}

// ---------- Échange des bords entre voisins ----------

void exchange_borders(grid_local *G, MPI_Comm comm,
                      border_cell *top_border,
                      border_cell *bottom_border) {
    int my_rank, comm_size;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &comm_size);

    int up   = (my_rank == 0) ? MPI_PROC_NULL : my_rank - 1;
    int down = (my_rank == comm_size - 1) ? MPI_PROC_NULL : my_rank + 1;

    int nx = G->nx;
    int ny = G->ny_local;

    border_cell *send_top = malloc(nx * sizeof(border_cell));
    border_cell *send_bot = malloc(nx * sizeof(border_cell));

    for (int j = 0; j < nx; j++) {
        // Ligne du haut locale
        cellule *c_top = &G->act[0][j];
        send_top[j].nb_S = send_top[j].nb_E = send_top[j].nb_I = send_top[j].nb_R = 0;
        for (int k = 0; k < c_top->nb_indiv; k++) {
            switch (c_top->indiv[k]->status) {
                case S: send_top[j].nb_S++; break;
                case E: send_top[j].nb_E++; break;
                case I: send_top[j].nb_I++; break;
                case R: send_top[j].nb_R++; break;
                default: break;
            }
        }

        // Ligne du bas locale
        cellule *c_bot = &G->act[ny - 1][j];
        send_bot[j].nb_S = send_bot[j].nb_E = send_bot[j].nb_I = send_bot[j].nb_R = 0;
        for (int k = 0; k < c_bot->nb_indiv; k++) {
            switch (c_bot->indiv[k]->status) {
                case S: send_bot[j].nb_S++; break;
                case E: send_bot[j].nb_E++; break;
                case I: send_bot[j].nb_I++; break;
                case R: send_bot[j].nb_R++; break;
                default: break;
            }
        }
    }

    MPI_Request reqs[4];

    MPI_Isend(send_top,   nx * sizeof(border_cell), MPI_BYTE, up,   0, comm, &reqs[0]);
    MPI_Isend(send_bot,   nx * sizeof(border_cell), MPI_BYTE, down, 1, comm, &reqs[1]);
    MPI_Irecv(top_border, nx * sizeof(border_cell), MPI_BYTE, up,   1, comm, &reqs[2]);
    MPI_Irecv(bottom_border, nx * sizeof(border_cell), MPI_BYTE, down, 0, comm, &reqs[3]);

    MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);

    free(send_top);
    free(send_bot);
}

// ---------- Étape locale : déplacement + infection + évolution ----------

void next_step_local(grid_local *G,
                     border_cell *top_border,
                     border_cell *bottom_border) {
    int nx = G->nx;
    int ny = G->ny_local;

    cellule **act  = G->act;
    cellule **next = G->next;

    // 1) Vider next (sans free des buffers indiv)
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            next[i][j].nb_indiv = 0;
            next[i][j].indiv = NULL;

        }
    }

    // 2) Déplacement local (bloqué aux bords locaux)
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            if (act[i][j].nb_indiv == 0) continue;

            for (int k = 0; k < act[i][j].nb_indiv; k++) {
                individus *ind = act[i][j].indiv[k];

                int dx = rand_int(-1, 1);
                int dy = rand_int(-1, 1);

                int ni = i + dy;
                int nj = (j + dx + nx) % nx;

                if (ni < 0)   ni = 0;
                if (ni >= ny) ni = ny - 1;

                cellule *c = &next[ni][nj];
                int pos = c->nb_indiv;

                if (pos == 0) {
                    if (c->indiv == NULL)
                        c->indiv = malloc(sizeof(individus *));
                } else {
                    c->indiv = realloc(c->indiv, (pos + 1) * sizeof(individus *));
                }

                c->indiv[pos] = ind;
                c->nb_indiv++;
            }
        }
    }

    // 3) Infection + évolution sur next
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            if (next[i][j].nb_indiv == 0) continue;

            int infect_voisin = 0;
            int suseptible = 0;

            // Comptage dans la cellule
            for (int k = 0; k < next[i][j].nb_indiv; k++) {
                if (next[i][j].indiv[k]->status == S) suseptible++;
                if (next[i][j].indiv[k]->status == I) infect_voisin++;
            }

            if (suseptible > 0) {
                // Comptage dans les 8 voisins (en tenant compte des bords MPI)
                for (int p = -1; p <= 1; p++) {
                    for (int q = -1; q <= 1; q++) {
                        int ni = i + p;
                        int nj = (j + q + nx) % nx;

                        if (ni < 0) {
                            border_cell bc = top_border[nj];
                            infect_voisin += bc.nb_I;
                        } else if (ni >= ny) {
                            border_cell bc = bottom_border[nj];
                            infect_voisin += bc.nb_I;
                        } else {
                            for (int r = 0; r < next[ni][nj].nb_indiv; r++) {
                                if (next[ni][nj].indiv[r]->status == I)
                                    infect_voisin++;
                            }
                        }
                    }
                }

                // Infection des S
                for (int k = 0; k < next[i][j].nb_indiv; k++) {
                    individus *ind = next[i][j].indiv[k];
                    if (ind->status == S && infection(infect_voisin)) {
                        ind->status = E;
                        ind->tps_in_status = 0.0f;
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
                            ind->tps_in_status = 0.0f;
                        } else ind->tps_in_status += 1.0f;
                        break;
                    case I:
                        if (ind->tps_in_status >= ind->dI) {
                            ind->status = R;
                            ind->tps_in_status = 0.0f;
                        } else ind->tps_in_status += 1.0f;
                        break;
                    case R:
                        if (ind->tps_in_status >= ind->dR) {
                            ind->status = S;
                            ind->tps_in_status = 0.0f;
                        } else ind->tps_in_status += 1.0f;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    cellule **tmp = G->act;
    G->act = G->next;
    G->next = tmp;

    G->temps++;
}

void buffer_init(indiv_buffer *buf) {
    buf->data = NULL;
    buf->count = 0;
    buf->capacity = 0;
}


void buffer_push(indiv_buffer *buf, individus *ind) {
    if (buf->count == buf->capacity) {
        buf->capacity = (buf->capacity == 0) ? 8 : buf->capacity * 2;
        buf->data = realloc(buf->data, buf->capacity * sizeof(individus));
    }
    buf->data[buf->count++] = *ind;   // copie de la structure
}


void send_buffer(indiv_buffer *buf, int dest, int tag, MPI_Comm comm) {
    MPI_Send(&buf->count, 1, MPI_INT, dest, tag, comm);
    if (buf->count > 0) {
        MPI_Send(buf->data, buf->count * sizeof(individus), MPI_BYTE, dest, tag+1, comm);
    }
}


void recv_buffer(indiv_buffer *buf, int src, int tag, MPI_Comm comm) {
    MPI_Recv(&buf->count, 1, MPI_INT, src, tag, comm, MPI_STATUS_IGNORE);

    if (buf->count > 0) {
        buf->capacity = buf->count;
        buf->data = malloc(buf->count * sizeof(individus));
        MPI_Recv(buf->data, buf->count * sizeof(individus), MPI_BYTE, src, tag+1, comm, MPI_STATUS_IGNORE);
    } else {
        buf->data = NULL;
        buf->capacity = 0;
    }
}


void insert_received_individuals(grid_local *G, indiv_buffer *buf, int row) {
    int nx = G->nx;

    for (int k = 0; k < buf->count; k++) {
        individus *ind = malloc(sizeof(individus));
        *ind = buf->data[k];

        int j = rand_int(0, nx - 1);  // colonne aléatoire

        cellule *c = &G->next[row][j];
        int pos = c->nb_indiv;

        if (pos == 0)
            c->indiv = malloc(sizeof(individus *));
        else
            c->indiv = realloc(c->indiv, (pos + 1) * sizeof(individus *));

        c->indiv[pos] = ind;
        c->nb_indiv++;
    }
}

void next_step_local_mg(grid_local *G,
                     border_cell *top_border,
                     border_cell *bottom_border,
                     MPI_Comm comm) {

    int my_rank, comm_size;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &comm_size);

    int up   = (my_rank == 0) ? comm_size - 1 : my_rank - 1;
    int down = (my_rank == comm_size - 1) ? 0 : my_rank + 1;

    int nx = G->nx;
    int ny = G->ny_local;

    cellule **act  = G->act;
    cellule **next = G->next;

    // ----------------- Buffers de migration (structures sérialisées) -----------------
    indiv_buffer send_up, send_down;
    buffer_init(&send_up);
    buffer_init(&send_down);

    // ----------------- 1) Vider next -----------------
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            next[i][j].nb_indiv = 0;
            next[i][j].indiv = NULL;
        }
    }

    // ----------------- 2) Déplacement + migration -----------------
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {

            cellule *c = &act[i][j];

            for (int k = 0; k < c->nb_indiv; k++) {

                individus *ind = c->indiv[k];

                int dx = rand_int(-1, 1);
                int dy = rand_int(-1, 1);

                int ni = i + dy;
                int nj = (j + dx + nx) % nx;

                // Migration vers le haut
                if (ni < 0) {
                    buffer_push(&send_up, ind);

                    // retirer l'individu de la cellule source
                    c->indiv[k] = c->indiv[c->nb_indiv - 1];
                    c->nb_indiv--;
                    k--; // re-tester la même position
                    continue;
                }

                // Migration vers le bas
                if (ni >= ny) {
                    buffer_push(&send_down, ind);

                    // retirer l'individu de la cellule source
                    c->indiv[k] = c->indiv[c->nb_indiv - 1];
                    c->nb_indiv--;
                    k--;
                    continue;
                }

                // Déplacement local (reste dans la bande)
                cellule *dest = &next[ni][nj];
                int pos = dest->nb_indiv;

                if (pos == 0)
                    dest->indiv = malloc(sizeof(individus *));
                else
                    dest->indiv = realloc(dest->indiv, (pos + 1) * sizeof(individus *));

                dest->indiv[pos] = ind;
                dest->nb_indiv++;
            }
        }
    }

    // ----------------- 3) Migration MPI -----------------
    indiv_buffer recv_up, recv_down;
    buffer_init(&recv_up);
    buffer_init(&recv_down);

    // Envoi des buffers (count + data)
    if (up != MPI_PROC_NULL)
        send_buffer(&send_up, up,   100, comm);
    if (down != MPI_PROC_NULL)
        send_buffer(&send_down, down, 200, comm);

    // Réception des buffers
    if (up != MPI_PROC_NULL)
        recv_buffer(&recv_up,   up,   200, comm);
    if (down != MPI_PROC_NULL)
        recv_buffer(&recv_down, down, 100, comm);

    // ----------------- 4) Insertion des individus reçus -----------------
    if (up != MPI_PROC_NULL)
        insert_received_individuals(G, &recv_up,   0);      // ligne du haut
    if (down != MPI_PROC_NULL)
        insert_received_individuals(G, &recv_down, ny - 1); // ligne du bas

    // ----------------- 5) Infection + évolution (ton cœur SEIR) -----------------
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {

            if (next[i][j].nb_indiv == 0)
                continue;

            int infect_voisin = 0;
            int suseptible = 0;

            // Comptage dans la cellule
            for (int k = 0; k < next[i][j].nb_indiv; k++) {
                if (next[i][j].indiv[k]->status == S) suseptible++;
                if (next[i][j].indiv[k]->status == I) infect_voisin++;
            }

            // Comptage dans les voisins
            if (suseptible > 0) {
                for (int p = -1; p <= 1; p++) {
                    for (int q = -1; q <= 1; q++) {

                        int ni = i + p;
                        int nj = (j + q + nx) % nx;

                        if (ni < 0) {
                            infect_voisin += top_border[nj].nb_I;
                        }
                        else if (ni >= ny) {
                            infect_voisin += bottom_border[nj].nb_I;
                        }
                        else {
                            for (int r = 0; r < next[ni][nj].nb_indiv; r++) {
                                if (next[ni][nj].indiv[r]->status == I)
                                    infect_voisin++;
                            }
                        }
                    }
                }

                // Infection des S
                for (int k = 0; k < next[i][j].nb_indiv; k++) {
                    individus *ind = next[i][j].indiv[k];
                    if (ind->status == S && infection(infect_voisin)) {
                        ind->status = E;
                        ind->tps_in_status = 0.0f;
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
                            ind->tps_in_status = 0.0f;
                        } else ind->tps_in_status += 1.0f;
                        break;

                    case I:
                        if (ind->tps_in_status >= ind->dI) {
                            ind->status = R;
                            ind->tps_in_status = 0.0f;
                        } else ind->tps_in_status += 1.0f;
                        break;

                    case R:
                        if (ind->tps_in_status >= ind->dR) {
                            ind->status = S;
                            ind->tps_in_status = 0.0f;
                        } else ind->tps_in_status += 1.0f;
                        break;

                    default:
                        break;
                }
            }
        }
    }

    // ----------------- 6) Swap des grilles -----------------
    cellule **tmp = G->act;
    G->act = G->next;
    G->next = tmp;

    G->temps++;
}
