/!\ Il faudrais analyser la diference cause par la fonction 
send_recv_replace d'apres l'analyse que j'avais effectue sur la
mult bcast a ma sauce, elle n'etait pas optimal en temps


Il serrait aussi interesant de voir la difference entre la boucle 
de calcul avec des indice precalculer et avec les indice calcule a 
chaque fois

/!\ Il faudrais dans les programme qui le font, suprimer la derniere 
communication MPI qui ne serra pas utilisé vu qu'il n'y pas d'autre itération apres

A benchmarker:  
les fonctions seqentielles avec avx et les comparer
comparer les gain de SUMMA_vect par raport a la version non vectoriser
et aussi regarder l'incidence des taille de block lors du blocking
sur les temps d'execution et la conso d'energie.
en O3


recompiler :
    gcc mult_mat_seq_blocking.c ../../../include/sources/affichage.c ../../../include/sources/energy_monitor.c ../../../include/sources/gest_arg.c ../../../include/sources/resize.c ../../../include/sources/for_fox.c ../../../include/sources/lecture_mat.c -O3 -march=native -funroll-loops -ftree-vectorize -o seq_auto_vect


    gcc mult_mat_seq_blocking.c ../../../include/sources/affichage.c ../../../include/sources/energy_monitor.c ../../../include/sources/gest_arg.c ../../../include/sources/resize.c ../../../include/sources/for_fox.c ../../../include/sources/lecture_mat.c -O3 -fno-tree-vectorize -o seq_pure


regarder la diference de temps et energie entre :
    la verision seq 03 auto-vectoriser
    la version non vectorisé
    le version vectorisé muanuelement 

faire des perf stat sur ces deux version sequentielle pour comprendre 
pk la plus optimisé en theorie est 10% plus lente