# Récapitulatif énergie - Ryzen AI 7 350

## Ce qui est disponible

Machine :
- AMD Ryzen AI 7 350
- 8 cœurs / 16 threads

Compteur énergétique détecté :
- /sys/class/powercap/intel-rapl:0/energy_uj

Domaine :
- package-0

Valeur maximale :
- 65532610987 µJ
- ≈ 65532.6 J

## Ce que mesure le compteur

Énergie du package CPU complet :
- cœurs
- caches
- logique interne du processeur

Unité :
- microjoules (µJ)

Conversion :
- Joules = valeur_uj / 1e6

## Protocole utilisé

Lancement programme

MPI_Barrier

Lecture temps_global_début (MPI_Wtime) / clock_get_t()

Lecture énergie_global_début

Initialisation / préparation des données

MPI_Barrier

Lecture temps_algo_début (MPI_Wtime) / clock_get_t()

Lecture énergie_algo_début

Boucle de Calcul principale

MPI_Barrier

Lecture temps_algo_fin (MPI_Wtime) / clock_get_t()

Lecture énergie_algo_fin

MPI_Barrier

Lecture temps global fin

Lecture énergie global fin

## Calculs

Temps :

T = Tfin - Tdebut

Énergie :

E = (Efin - Edebut) / 1e6 --> convertion en joule

Puissance moyenne :

P = E / T

Speedup :

S = Tseq / Tp

Efficacité :

Eff = S / nb_processus

Energy Delay Product :

EDP = E * T

## Campagne de tests

Tailles :
- 256
- 512
- 1024
- 2048
- 4096
- 80192

Processus (topologie classique):
- 2
- 4
- 8
- 12
- 16

OU (pour topologie grille) : 

- 4
- 9
- 16


Algorithmes :
- Séquentiel
- 1D
- Blocs
- Fox
- Cannon
- SUMMA
- Cannon + blocking
- SUMMA + blocking

Faire au moins 5 exécutions et calculer les moyennes (temps energie puissance).
