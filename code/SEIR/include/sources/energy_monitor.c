#include "../headers/energy_monitor.h"

long long read_energy_uj(void)
{
    FILE *f = fopen(RAPL_PATH, "r");
    if(f == NULL){
        printf("on a pas ouvers l'energie\n");
    }
    long long e = 0;

    if(f == NULL)
        return -1;

    int error = fscanf(f, "%lld", &e);
    fclose(f);

    return e;
}

void benchmark_end(BenchResult *result,
                   long long start_energy_uj,
                   double start_time_s,
                   long long end_energy_uj,
                   double end_time_s)
{
    long long delta_uj;


    if(end_energy_uj >= start_energy_uj)
        delta_uj = end_energy_uj - start_energy_uj;
    else
        delta_uj = (RAPL_MAX_UJ - start_energy_uj) + end_energy_uj;

    result->time_s = end_time_s - start_time_s;
    result->energy_j = delta_uj / 1000000.0;
    result->avg_power_w = result->energy_j / result->time_s;

}


