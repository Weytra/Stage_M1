library(ggplot2)
library(tidyverse)
library(plotly)
library(dplyr)


write.csv(Mean_res, "infect_global.csv", row.names = TRUE)


#generation des bon nom dans les csv

infect_mpi_gl = rbind(infect_mpi_big,infect_mpi,infect_mpi_C_friendly.)

infect_new_seq = rbind(infect_seq_vect,infect_seq_base,infect_seq_no_vect)

new_mpi = rbind(infect_mpi_base,infect_mpi_no_vect,infect_mpi_vect)

new_omp = rbind(infect_omp_0_friendly,infect_omp_0_no_friendly,infect_omp_2friendly)

seq_gl = rbind(infect_seq,infect_seq_A_vect_forced,infect_seq_big,infect_seq_C_friendly,infect_seq_C_NO_friendly,infect_seq_no_vect)

hybrid = rbind(infect_hybrid_MPI_OMP,infect_hybrid_MPI_OMP_2)

infect_omp_gl = rbind(infect_omp,infect_omp_2,infect_omp_big,infect_omp_C_friendly.,infect_omp_friendly)


mpi_gl_2 = rbind(infect_mpi,infect_mpi_base,infect_mpi_big,infect_mpi_C_friendly,infect_mpi_C_friendly.,infect_mpi_C_NO_friendly,new_mpi)


seq_gl_2 = rbind(infect_seq_C_friendly,infect_seq_C_NO_friendly,infect_seq_A_vect_forced,infect_seq_base,infect_seq_no_vect,infect_seq_vect)

omp_gl_2 = rbind(infect_omp,infect_omp_0_friendly,infect_omp_0_no_friendly,infect_omp_2,infect_omp_2friendly,infect_omp_C_friendly.,infect_omp_friendly)


Bench_2 = omp_gl_2

#avec les nouveau test de bench en plus
infect_mpi_gl_2 = rbind(infect_mpi,infect_mpi_big,infect_mpi_C_friendly,infect_mpi_C_friendly.,infect_mpi_C_NO_friendly)

mpi_gl = infect_mpi_gl %>%mutate(omp_threads = 1)

infect_gl = rbind(omp_gl,mpi_gl,seq_gl,infect_hybrid_MPI_OMP,infect_hybrid_MPI_OMP_2,infect_mpi_big_it)

seq_gl = seq_gl %>% mutate(np = 1) %>% mutate(omp_threads = 1)
infect_gl = infect_gl %>% filter(exec != seq_big_20k)

fix_types <- function(df) {
  df |> mutate(
    np = as.integer(np),
    omp_threads = as.integer(omp_threads),
    exec = as.character(exec),
    opt = as.character(opt)
  )
}

omp_gl  <- fix_types(omp_gl)
mpi_gl  <- fix_types(mpi_gl)
seq_gl  <- fix_types(seq_gl)
hyb_gl  <- fix_types(infect_hybrid_MPI_OMP)


#on a a present les 3 type principaux avec les donnees suivantes:
# infect_seq_gl
# infect_mpi_gl
# infect_omp_gl



Bench_3 = rbind(mpi,mpi_avx,seq_blk,seq);
Bench_2 = infect_seq_gl
Bench_2 = infect_mpi_gl_2
Bench_2 = infect_omp_gl
Bench_2 = infect_hybrid_MPI_OMP
Bench_2 = infect_gl
Bench_2 = hybrid
Bench_2 = infect_mpi_big_800it

Bench_2 = seq_gl_2
Bench_2 = rbind(hybrid,infect_hybrid_MPI_OMP,infect_hybrid_MPI_OMP_2)
write.csv(Mean_res, "seq_global_2.csv", row.names = TRUE)

#M
Mean_res = Bench_2 %>% group_by(exec,opt,nb_indiv,size,pct_infect) %>%
  summarise(G_time = mean(GT),G_energie= mean(GE),G_power = mean(GP),A_time = mean(AT),A_energie = mean(AE),A_power = mean(AP),.groups =  "drop");


Mean_res = Bench_2 %>% group_by(exec,opt,nb_indiv,size,pct_infect,np,omp_threads) %>%
  summarise(G_time = mean(GT),G_energie= mean(GE),G_power = mean(GP),A_time = mean(AT),A_energie = mean(AE),A_power = mean(AP),.groups =  "drop");



Mean_res = Mean_res %>% filter(exec != "seq_blk")


Mean_res %>% filter(exec =="seq") %>% 
  select(exec,opt,size,nb_indiv,nb_infect,G_time,G_energie,G_power) %>% 
  ggplot(aes(x = size,y = G_time,color = factor(opt)))+geom_line()+geom_point()

### temps global
p = Mean_res %>% 
  filter(nb_indiv <70000) %>% 
  select(exec, np, block,size, G_time, G_energie, G_power) %>% 
  ggplot(aes(
    x = size,
    y = G_time,
    color = interaction(np,block, sep ="_"),
    group = np,
    text = paste(
      "Algo :", exec,
      "<br> size : ", size,
      "<br>np :", np,
      "<br>Temps :", round(G_time, 3), "s",
      "<br>Énergie :", round(G_energie, 3), "J",
      "<br>power :", round(G_power, 3),
      "<br> size = ", size
    )
  )) +
  facet_wrap(~ exec) +
  scale_x_log10() +
  scale_y_log10() +
  geom_line() +
  geom_point()

ggplotly(p, tooltip = "text")


##############
p <- Mean_res %>%
  filter(nb_indiv <80000) %>% 
  filter(pct_infect == 30) %>% 
  filter(exec %in% c("seq_C_NO_friendly","seq_C_friendly","seq_nv")) %>% 
  ggplot(aes(
    x = size,
    y = A_time,
    color = factor(nb_indiv),
    group = nb_indiv,
    text = paste(
      "Algo :", exec,
      "<br>opt :", opt,
      "<br>size :", size,
      "<br>nb_indiv :",nb_indiv,
      "<br>% infectés :", pct_infect,
      "<br>Temps :", round(A_time, 3), "s",
      "<br>Énergie :", round(A_energie, 3), "J",
      "<br>Power :", round(A_power, 3)
    )
  )) +
  facet_wrap(~ exec) +
  geom_line() +
  geom_point() +
  theme_minimal()

ggplotly(p, tooltip = "text")



p <- Mean_res %>%
  #filter(omp_threads == 2) %>% 
  filter(exec %in% c("seq","omp_v2","mpi__big","hybrid_2")) %>% 
  filter(pct_infect ==30) %>% 
  filter(nb_indiv<20001) %>% 
  filter(np %in% c(1,8,16)) %>% 
  filter(size <=800) %>% 
  filter(opt == "O3") %>% 
  ggplot(aes(
    x = size,
    y = A_time,
    color = factor(nb_indiv),
    group = np,
    text = paste(
      "Algo :", exec,
      "<br>opt :", opt,
      "<br>size :", size,
      "<br>omp : ",omp_threads,
      "<br>np :",np,
      "<br>nb_indiv :",nb_indiv,
      "<br>% infectés :", pct_infect,
      "<br>Temps :", round(A_time, 3), "s",
      "<br>Énergie :", round(A_energie, 3), "J",
      "<br>Power :", round(A_power, 3)
    )
  )) +
  facet_wrap(~ exec) +
  geom_line() +
  geom_point() +
  theme_minimal()


ggplotly(p, tooltip = "text")

Mean_res_plot <- Mean_res %>%
  filter(nb_indiv < 80000) %>% 
  filter(pct_infect == 30) %>% 
  filter(size < 1200) %>% 
  #filter(exec %in% c("","seq_C_friendly")) %>% 
  mutate(
    tooltip = paste(
      "Algo :", exec,
      "<br>opt :", opt,
      "<br>size :", size,
      "<br>nb_indiv :", nb_indiv,
      "<br>% infectés :", pct_infect,
      "<br>Temps :", round(A_time, 3), "s",
      "<br>Énergie :", round(A_energie, 3), "J",
      "<br>Power :", round(A_power, 3)
    )
  ) %>%
  pivot_longer(
    cols = c(A_time, A_energie),
    names_to = "metric",
    values_to = "value"
  ) %>%
  mutate(
    metric = factor(metric, levels = c("A_time", "A_energie"))
  )

p <- Mean_res_plot %>%
  ggplot(aes(
    x = size,
    y = value,
    color = factor(nb_indiv),
    group = nb_indiv,
    text = tooltip
  )) +
  facet_grid(metric ~ exec, scales = "free_y") +
  geom_line() +
  geom_point() +
  theme_minimal()

ggplotly(p, tooltip = "text")


