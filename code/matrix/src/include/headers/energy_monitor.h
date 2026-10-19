#ifndef ENERGY_MONITOR_H
#define ENERGY_MONITOR_H

#include <time.h>
#include <stdio.h>


#define RAPL_PATH "/sys/class/powercap/intel-rapl:0/energy_uj"
#define RAPL_MAX_UJ 65532610987LL

typedef struct
{
    double time_s;
    double energy_j;
    double avg_power_w;
} BenchResult;      

typedef struct
{
    BenchResult global;
    BenchResult algo;
} FullBenchResult;

long long read_energy_uj(void);

void benchmark_end(BenchResult *result,
                   long long start_energy_uj,
                   double start_time_s,
                   long long end_energy_uj,
                   double end_time_s);

static inline double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    //CLOCK_MONOTONIC_RAW sous linux
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}


#endif

