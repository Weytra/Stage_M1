library(ggplot2)
library(tidyverse)
library(plotly)

M = resultat_global


Mean_res = M %>% group_by(exec,opt,np,size) %>%
  summarise(G_time = mean(global_time),G_energie= mean(global_energy),G_power = mean(global_power),A_time = mean(algo_time),A_energie = mean(algo_energy),A_power = mean(algo_power),.groups =  "drop");

#affiche les temps d'execution de SUMMA
Mean_res %>% filter(exec =="SUMMA_",opt == "O0") %>% 
  select(exec,opt,np,size,G_time,G_energie,G_power) %>% 
  ggplot(aes(x = size,y = G_time,color = factor(np)))+geom_line()+geom_point()



Mean_res %>% filter(exec =="SUMMA_") %>% 
  select(exec,opt,np,size,G_time,G_energie,G_power) %>% 
  ggplot(aes(x = size,y = G_time,color = interaction(np,opt, sep ="_")))+geom_line()+geom_point()

# donnee interesante a connaitre/visualiser : 
# courbes des temps d'execution/energie pour l'algo/global en fonction de la taille pour un nombre de procesus donnee
# temps d'execution/energie pour l'algo/global en fct du nombre de processus pour une taille donnee (et variation du meilleur celon la taille)

# analyser quel est la methode d'optimisation est la plus rapide et ci ca varie celon les algos

# le meilleur algo et opti par taille
size_ = 1024
Mean_res %>% filter(size == size_) %>% filter(G_time == min(G_time))
# 4096 : send receve remplace Ofast 8
# 2048 :mult_send_rcv_replace Ofast 16
#mult_bcast_blk O3        8  1024

Mean_res %>% filter(size == size_) %>% filter(A_time == min(A_time))
# mult_bcast_blk O3 16 4096 & 2048
# mult_bcast_blk Ofast    16  1024 

Mean_res %>% filter(size == size_) %>% filter(G_energie == min(G_energie))
#4096 : mult_bcast_blk OFast 4
#2048 : mult_send_rcv_replace O3  4
# mult_bcast_blk O3        8  1024 

Mean_res %>% filter(size == size_) %>% filter(A_energie == min(A_energie))
# mult_bcast_blk O3 16  4096 & 2048
#                   8 1024

Mean_res %>% filter(size == size_) %>% filter(G_power == min(G_power))
#fox 2 O3 4     4096 & 2048
# mult_bcast_blk O2        4  1024 

Mean_res %>% filter(size == size_) %>% filter(A_power == min(A_power))
#fox 2 O3 4       4096 & 2048
#mult_bcast_blk O2        4  1024

# si l'on regarde uniquement les meilleur temps on constate que 3 algo resorte, 
# dont deux principalement pour les taille de matrice de 4096:
# la mlt send recv remplace en ofast avec 8 procesus est celle qui a ete la plus rapide de toute 
# sur le temps global d'execution, en revanche si on compte uniquement le temps de calcul
# intreseque a l'algorithme (hors initialisation des donnee et preparation de mpi) c'est l'algo
# mult bcast blk en O3 avec 16 processus qui ressort.
# concernant la consomation energetique minimal pour cette taille de matrice il advient egalement
# qu'il s'agit de ce programme la qui est le plus econome, mais pas avec le meme nombre de proc
# ni la meme optimisationd e compilation
# enfin en terme de puissance c'est en locurence fox_2 (fox avec ma propre implementtaion 
# de la grille mpi) en 03 avec 4 proc qui s'avere etre la plus econome en watt consomé

# apres denombrement on constate que sur ces 3 taille annalysé il y a 11 cas ou bcast est le
# meilleur algos, suivi par fox avec 4 points, et send_recv_replace avec 3 points


# stabilite des temps et des cout energietique au seins d'une meme variante du programme (parmis les 5 repetition)





Mean_res %>%#filter(size != 4096) %>% 
  filter(exec == "fox_2") %>%
  ggplot(aes(x = size, y = G_time, color = factor(np))) +
  geom_line() +
  geom_point() +
  facet_wrap(~ opt)+
  scale_x_log10()+
  scale_y_log10()+
  labs(
    title = "Temps global – SUMMA",
    x = "Taille de matrice",
    y = "Temps (s)",
    color = "Nb de processus"
  ) +
  theme_minimal()
#  ___ SUMMA ____
# on remarque que le gain de temps n'est pas totalement correler a l'augmentation du nombre de processus
# le nombre de procesus optimal apparait etre 9, qui est environ deux fois plus rapide que 16,
# en revanche comme on pouvais s'y attendre, avec seulement 4 processus (le minimum pour utiliser summa)
# le temps est bien plus lent que pour les deux autres nombre de processus testé
# on constate egalement que la diference de temps celon le nombre de proc explose pour une taille de 4096, 
# avant cela la difference existe mais de facon moins significative (en absolue evidament mais egalement en proportion)
# Avec l'utilisation d'une echelle logarithmique on remarque que jusqua la taille 128 le nombre optimal de
# procesus est de 4, passé cette taile il passed a 9, on peux extrapoler que a partie d'une 
# autre taille les 16 processus serront plus performant, peux etre pour des taille de 8192 ou 16384


# __ CANNON __ 
# idem en 4096 
# en revanche en taille 2048 et en O2 et O0 16 procesus est effectivement plus rapide

# il semble apparaitre que en O0 le nombre de 4 processus est plus rapide que 9 ou 16 proc
#importe la taille


Mean_res %>%
  ggplot(aes(x = size, y = G_time, color = factor(np))) +
  geom_line() +
  facet_wrap(~ exec) +
  scale_x_log10() +
  scale_y_log10() +
  theme_minimal()

Mean_res  %>% filter(opt == "O3") %>% 
  ggplot(aes(x = size, y = G_energie, color = factor(np))) +
  geom_line() +
  facet_wrap(~ exec) +
  scale_x_log10() +
  scale_y_log10() +
  theme_minimal()


Mean_res_aug = Mean_res %>%
  mutate(
    ratio_global = G_time / G_energie,
    ratio_algo   = A_time / A_energie
  )

Mean_res_aug %>%
  filter(opt == "O3") %>%
  ggplot(aes(x = size, y = ratio_algo, color = factor(np))) +
  geom_line() +
  geom_point() +
  scale_x_log10() +
  scale_y_log10() +
  facet_wrap(~ exec) +
  labs(
    title = "Ratio temps/énergie – SUMMA O3",
    x = "Taille de matrice",
    y = "Temps / Énergie",
    color = "np"
  )




# visualisation des algos avec le meilleur compris consomation & temps
Mean_res %>%#filter(size == 4096) %>% 
  ggplot(aes(x = G_time, y = G_energie, color = exec)) +
  geom_point(size = 2, alpha = 0.7) +
  scale_x_log10() +
  scale_y_log10() +
  labs(
    title = "Compromis Temps / Énergie",
    x = "Temps global (s)",
    y = "Énergie globale (J)",
    color = "Algorithme"
  ) +
  theme_minimal()

# on voit ici que les 3 algos type mlt* sont les plus interesant sous l'aspect temps & energie
# contrairement au algo qui utilisent une topologie de type grille (summa fox cannon) qui sont
#bien moins bon sur ce graphique la.





Mean_res_aug %>%
  filter(size == 4096) %>%
  ggplot(aes(x = np, y = ratio_global, color = opt)) +
  geom_line() +
  geom_point() +
  facet_wrap(~ exec) +
  labs(
    title = "Ratio temps/énergie – SUMMA (taille 4096)",
    x = "Nombre de processus",
    y = "Temps / Énergie",
    color = "Optimisation"
  )



#affiche le temps de chaque algo dans les diferentes optimisation et nombre de processus

p= Mean_res %>% filter(size == 4096) %>% filter(opt != "O0") %>% #filter(np %in% c(4)) %>% 
  ggplot(aes(x = exec,y = G_energie,color=opt,text = paste("Algo :", exec,"<br>Opt :", opt,"<br>np :", np,"<br>Temps :", round(G_time, 3), "s","<br>Énergie :", round(G_energie, 3), "J<br>power :", round(G_power, 3),"<br> size = ",size)))+
  geom_point(size = 2,alpha= 0.7)+
  facet_wrap(~ np)

ggplotly(p, tooltip = "text")


# ### ANALYSE en regardant le temps passé dans la partie algorithmique (le coeur du programme)
# On remarque une autre tendance pour O0, il semble que cette optimisation ai un faible impact
# sur la partie vraiment calculatorie du programme, ici en analysant le temps passe dans la 
# boucle principale du programme on remarque bien que O0 est a quelque exception pret vraiment plus
# mauvais que les autre optimisation et de facon vraiment significative
# les opt O3 et Ofast sont generalement tres poches.
# 
# en 2048 en moyenne 4.8s en grille, 4.2 mlt* avec 16p/ 12 vs 4 avec 4p
# 
# 
# ### analyse sur l'energie au seins de l'algo en lui meme.
# On retrouve les meme tendance que pour les temps d'execution. 
# Concernant les consomation energetique on retrouve des ecart considerable allant la aussi de 
# *3 avec 4 processus entre mlt* et grille a 15% pour a l'aventage des mlt* avec 16p
# Avec une grille de 4096, on ce retrouve aussi avec une diference considerable allant jusqua *10 
# avec 4 processus, en ces temps ou l'energie devient une denré rare il apparait que certains algos
# et sourtout nombre de processus sont preferable a d'autre.
# Avec 8_9 processus on est a environ 200J pour les mlt* contre 300J pour les grille
# Avec 16 processus la tendance pour la grille s'inverse et on constate une forte augmentation a 
# 700J pour les grille contre une conso plus stable a 275J pour les mlt*

# Avec cette taille de 4096 je remarque que :
#   pour les grille : la consomation diminue de 4 a 9 avant de reaugmenter pour 16;
#   pour les mlt* la conso augmente avec le nombre de processus, passant de 150J pour 4p, a 
#   175J pour 8p, et enfin 200J pour 16p.
#   Alors que le temps d'execution reste fixe ! Donc il n'est pas interesant de faire tourner
#   ces programme avec beaucoup de processu, il faudrais tester avec 2 voir 1 seul procesus pour voir
#   le temps
# 
# 
# 

# Remarque importante general : 
#   on constate que les algos de type mlt* sont toujours plus rapide que les algos grille sur mon pc,
# de plus ces 3 algos sont tres simillaire sur les temps d'execution, et le temps d'execution ne 
# change que tres peu en fonction du nombre de processus, ce qui est surprenant et meriterais 
# d'etre creusé ci j'ai le temps de le faire.
# RQ importante sur le temps de calcul uniquement : 
# De l'autre coté, les algos de type grille sont eux plus logique dans l'evolution du temps en 
# fonction du nombre de processu. en effet pour une taille par ex de 2048 ils sont aux allentour de
# 13s d'execution avec 4 proc, contre 5s avec 9 et 5.5 avec 16 proc, donc plus de processus n'est
# pas toujours mieux.


### ANALYSE de la puissance
# 
# avec taille de 4096 on remarque une certaine constance entre toutes les version allant de 6w a 13w,
# avec entre 6 et 8 pour np = 4, 8.7 et 11.6 pour 8_9 proc, et entre 10 et 13w pour 16 processus


### AU global en regardant les temps d'execution moyen !
# 
# # concernant le temps d'execution global on constate a nouveau que les algos de type mlt* donc sans
# usage de grille son les plus rapide, et de beaucoup, allant de -20s pour ces implementation (dans leur meilleur opt),
# contre + 60s pour les algos de type grille, soit 3 fois moins rapide.
# Avec 8_9 proce on constate toujours que les mlt* sont meilleur, mais de facon moins extreme,
# on est (encore ) a 19 pour les mlt* contre 28s pour les topologie grille
# On y vois ici que l'optimisation O0 est toujours la pire, et de loin.
# avec cette fois 4 procesus opt O0 devient la plus constante a travers la totalite des algo,
# etant le meilleur des opti en grille (et de loin /2) a la pire dasn les mlt* (*2)
# 
# avec une taille de 2048, opt O0 est tres mauvais pour 18 des versions et tres bon pour les 4
# version sous grille avec avec 9 processus.
# 
# ce qui serrait a verifier : es ce que dans les version ou O0 est performant est du au donnee
# d'entré ou bien a la forme (ie liée aux matrices ou a la version executé)
# 
# Analyse concernant les OPT :
#   O0 est extrêmement variable, generalement cette optimisation est plutot mauvaise par rapport
#   aux autres cependant
#   il s'avere que c'est la meilleur dans certaines execution et de tres loin, je ne parviens pas
#   a l'expliquer pour le moment. L'exemple le plus criant est en 4096 où avec 4 proc
#   pour les algo de type grille on a un temps 2x plus court que pour les autre opti, mais pour
#   les algo mlt* opt O0 est 2x plus lent.
# 
#   Ofast quand a lui a un profil inversé a O0, il est generalement plus rapide que la moyenne,
#   parfois meme de beaucoup mais dans 10% des cas il ce trouve etre le pire, mais pas de facon
#   aussi extreme que les O0, il est juste mauvais, mais sans aller dans l'extreme.
# 
#   O2 et O3 sont simillaire, avec une faible variabilité et un avantage de l'ordre de 10% pour O2




cor(Mean_res$A_time, Mean_res$A_energie, method = "pearson")
cor(Mean_res$G_time, Mean_res$G_energie, method = "pearson")
 # 0.97 dans les deux cas ce qui montre une forte correlation entre les deux ce que est
 # parfaitement logique car plus longtemps l'algo tourne plus il va consomer de l'energie.



p= Mean_res %>% filter(size == 4096) %>% filter(opt != "O0") %>% filter(exec == "mlt_mat_prl_blk" | exec == "mult_bcast_blk" | exec == "mult_send_rcv_replace") %>% #filter(np %in% c(4)) %>% 
  ggplot(aes(x = exec,y = G_energie,color=opt,text = paste("Algo :", exec,"<br>Opt :", opt,"<br>np :", np,"<br>Temps :", round(G_time, 3), "s","<br>Énergie :", round(G_energie, 3), "J<br>power :", round(G_power, 3),"<br> size = ",size)))+
  geom_point(size = 2,alpha= 0.7)+
  facet_wrap(~ np)

ggplotly(p, tooltip = "text")

# cette derniere visualisation nous permet de voir que l'algo bcast est le plus peformant et
# que O3 et Ofast sont les deux meilleur optimisation en terme d'energi mais aussi de temps
#