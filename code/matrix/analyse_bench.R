library(ggplot2)
library(tidyverse)
library(plotly)
library(dplyr)

M = gloabl_bench
M = Global_result_mpi

files <- list.files("C:\\Users\\pluch\\Desktop\\Fac\\M1\\S2\\Stage\\code\\src\\bench_4\\results", pattern = "\\.csv$", full.names = TRUE)

mpi_avx_fs <- read_csv("C:\\Users\\pluch\\Desktop\\Fac\\M1\\S2\\Stage\\code\\src\\bench_2\\results\\mpi_avx_fs")

seq_blk = seq_blk %>% mutate(np = 1)

seq_blk = seq_blk %>% mutate(np = 1)

seq_pure$exec <- "seq_pure"


Bench_3 = rbind(mpi,mpi_avx,seq_blk,seq);
Bench_4 = Bench_3

#M
Mean_res = Bench_4 %>% group_by(exec,np,size,block) %>%
  summarise(G_time = mean(GT),G_energie= mean(GE),G_power = mean(GP),A_time = mean(AT),A_energie = mean(AE),A_power = mean(AP),.groups =  "drop");

Mean_res = Mean_res %>% filter(exec != "seq_blk")


#affiche les temps d'execution de SUMMA (rien de nouveau)
Mean_res %>% filter(exec =="mpi") %>% 
  select(exec,np,size,G_time,G_energie,G_power) %>% 
  ggplot(aes(x = size,y = G_time,color = factor(np)))+geom_line()+geom_point()

### temps global
p = Mean_res %>% 
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

### temps algo
p = Mean_res %>% 
  select(exec, np, block,size, A_time, A_energie, A_power) %>% 
  ggplot(aes(
    x = size,
    y = A_time,
    color = interaction(np,block, sep ="_"),
    group = np,
    text = paste(
      "Algo :", exec,
      "<br> size : ", size,
      "<br>np :", np,
      "<br>Temps :", round(A_time, 3), "s",
      "<br>Énergie :", round(A_energie, 3), "J",
      "<br>power :", round(A_power, 3),
      "<br> size = ", size
    )
  )) +
  facet_wrap(~ exec) +
  scale_x_log10() +
  scale_y_log10() +
  geom_line() +
  geom_point()

ggplotly(p, tooltip = "text")

write.csv(Mean_res, "Bench_3_Transpose.csv", row.names = TRUE)


Mean_res %>% filter(exec =="SUMMA_") %>% 
  select(exec,opt,np,size,G_time,G_energie,G_power) %>% 
  ggplot(aes(x = size,y = G_time,color = interaction(np,opt, sep ="_")))+geom_line()+geom_point()
