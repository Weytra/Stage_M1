library(ggplot2)
library(tidyverse)
df = resultat_global_total

df %>% filter(size == 4096) %>% group_by(exec) %>% filter(time == min(time))


df %>% filter(size == 128)

df %>% filter(size == 4096) %>% group_by(exec) %>%  filter(time == min(time)) %>% select(opt)

df %>% filter(size == 4096)  %>% group_by(exec) %>% filter(opt == "O3") %>% 
  ggplot()+aes(y = time,x = np,color = paste(exec,opt,sep="_"))+geom_point()+
  geom_line()



ggplot(df, aes(x = np, y = time, color = paste(exec,opt,sep="_"))) +
  geom_line() +
  geom_point() +
  facet_wrap(~ size, scales = "free_y") +
  theme_minimal() +
  labs(
    title = "Temps moyen en fonction du nombre de processus par taille de matrice",
    x = "nombre de processus",
    y = "Temps moyen (s)",
    color = "Version"
  )

ggplot(df, aes(x = block, y = avg_time, color = interaction(opt, size))) +
  geom_line() +
  geom_point() +
  theme_minimal() +
  labs(
    title = "Comparaison complète block × optimisation × taille",
    x = "Taille de bloc",
    y = "Temps moyen (s)",
    color = "Opt.Taille"
  )+
  theme(
    legend.position = "bottom",
    legend.direction = "horizontal",
    legend.box = "horizontal"
  )

max(result$size)

results %>% filter(size == max(size))%>% ggplot()+aes(x = block,y=avg_time,color = opt)+
  geom_line()+geom_point()+
  labs(title = "matrice 1024, tps d'exec en fct de la taille de block")



result %>% ggplot()+aes(x = size,y=avg_time,color = opt)+geom_point()+geom_line()
