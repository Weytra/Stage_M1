########################################
# COMPILATEURS
########################################
CC = gcc
MPICC = mpicc

########################################
# FLAGS
########################################
BASEFLAGS = -march=native -funroll-loops -Iinclude/headers
OMPFLAGS = -fopenmp
LDFLAGS = -lm

########################################
# OPTIMISATIONS À TESTER
########################################
OPTS = O0 O1 O2 O3 Ofast

########################################
# PARAMÈTRES BENCHMARK
########################################
SIZES = 100 #200 300
NB_INDIV = 5000 #10000 20000
INFECT_PCT = 10# 20 30 40 50 60
REPEAT = 1
MPI_PROCS = 2# 4 8


########################################
# DOSSIERS BINAIRES
########################################
BIN_DIR = bin
SEQ_DIR = $(BIN_DIR)/seq
OMP_DIR = $(BIN_DIR)/omp
MPI_DIR = $(BIN_DIR)/mpi

$(shell mkdir -p $(SEQ_DIR) $(OMP_DIR) $(MPI_DIR))

########################################
# SOURCES COMMUNES
########################################
COMMON_SRC = include/sources/Fonctions.c \
	include/sources/energy_monitor.c \
	include/sources/gest_arg.c

########################################
# GÉNÉRATION DES OBJETS OPTIMISÉS
########################################

# Compilation normale (gcc)
define COMPILE_OPT
$1_$2.o: $1.c
	$(CC) -$(2) $(BASEFLAGS) -c $$< -o $$@
endef

# Compilation MPI (mpicc)
define COMPILE_OPT_MPI
$1_$2.o: $1.c
	$(MPICC) -$(2) $(BASEFLAGS) -c $$< -o $$@
endef

# Appliquer aux fichiers communs
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT,include/sources/Fonctions,$(opt))))
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT,include/sources/energy_monitor,$(opt))))
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT,include/sources/gest_arg,$(opt))))

# Appliquer aux fichiers SEQ / OMP
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT,src/seq/Infect_seq,$(opt))))
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT,src/omp/Infect_omp,$(opt))))

# Appliquer aux fichiers MPI
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT_MPI,src/mpi/infect_mpi,$(opt))))
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT_MPI,include/sources/fonctions_mpi,$(opt))))

########################################
# EXÉCUTABLES OPTIMISÉS
########################################

# SEQ
SEQ_BINS = $(foreach opt,$(OPTS),$(SEQ_DIR)/infect_$(opt))

$(SEQ_DIR)/infect_%: src/seq/Infect_seq_%.o \
	include/sources/Fonctions_%.o \
	include/sources/energy_monitor_%.o \
	include/sources/gest_arg_%.o
	$(CC) -$* $(BASEFLAGS) $^ $(LDFLAGS) -o $@

# OMP
OMP_BINS = $(foreach opt,$(OPTS),$(OMP_DIR)/infect_omp_$(opt))

$(OMP_DIR)/infect_omp_%: src/omp/Infect_omp_%.o \
	include/sources/Fonctions_%.o \
	include/sources/energy_monitor_%.o \
	include/sources/gest_arg_%.o
	$(CC) -$* $(BASEFLAGS) $(OMPFLAGS) $^ $(LDFLAGS) -o $@


# MPI
MPI_BINS = $(foreach opt,$(OPTS),$(MPI_DIR)/mpi_exec_$(opt))

$(MPI_DIR)/mpi_exec_%: src/mpi/infect_mpi_%.o \
	include/sources/fonctions_mpi_%.o \
	include/sources/Fonctions_%.o \
	include/sources/energy_monitor_%.o \
	include/sources/gest_arg_%.o
	$(MPICC) -$* $(BASEFLAGS) $^ $(LDFLAGS) -o $@

########################################
# RÈGLE PRINCIPALE
########################################
all: $(SEQ_BINS) $(OMP_BINS) $(MPI_BINS)

########################################
# BENCHMARK SEQ AVEC CALCUL POURCENTAGE
########################################
run_bench_seq:
	mkdir -p results; \
	CSV=results/infect_seq.csv; \
	echo "exec,opt,size,nb_indiv,pct_infect,nb_infect,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for opt in $(OPTS); do \
		BIN=$(SEQ_DIR)/infect_$${opt}; \
		for n in $(SIZES); do \
			for indiv in $(NB_INDIV); do \
				for pct in $(INFECT_PCT); do \
					INF=$$(( indiv * pct / 100 )); \
					echo "=== SEQ $$opt $$n $$indiv $$pct% (INF=$$INF) ==="; \
					for r in $$(seq 1 $(REPEAT)); do \
						OUT=$$($$BIN $$n $$indiv $$INF 2>/dev/null); \
						GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
						GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
						GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
						AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
						AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
						AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
						echo "seq,$$opt,$$n,$$indiv,$$pct,$$INF,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
						sleep 1; \
					done; \
				done; \
			done; \
		done; \
	done


run_bench_omp:
	mkdir -p results; \
	CSV=results/infect_omp.csv; \
	echo "exec,opt,size,nb_indiv,pct_infect,nb_infect,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for opt in $(OPTS); do \
		BIN=$(OMP_DIR)/infect_omp_$${opt}; \
		for n in $(SIZES); do \
			for indiv in $(NB_INDIV); do \
				for pct in $(INFECT_PCT); do \
					INF=$$(( indiv * pct / 100 )); \
					echo "=== omp $$opt $$n $$indiv $$pct% (INF=$$INF) ==="; \
					for r in $$(seq 1 $(REPEAT)); do \
						OUT=$$($$BIN $$n $$indiv $$INF); \
						GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
						GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
						GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
						AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
						AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
						AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
						echo "omp,$$opt,$$n,$$indiv,$$pct,$$INF,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
						sleep 1; \
					done; \
				done; \
			done; \
		done; \
	done


run_bench_mpi:
	mkdir -p results; \
	CSV=results/infect_mpi.csv; \
	echo "exec,opt,np,size,nb_indiv,pct_infect,nb_infect,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for opt in $(OPTS); do \
	    for np in $(MPI_PROCS); do \
	        BIN=$(MPI_DIR)/mpi_exec_$${opt}; \
	        for n in $(SIZES); do \
	            for indiv in $(NB_INDIV); do \
	                for pct in $(INFECT_PCT); do \
	                    INF=$$(( indiv * pct / 100 )); \
	                    echo "=== MPI $$opt $$np proc $$n $$indiv $$pct% (INF=$$INF) ==="; \
	                    for r in $$(seq 1 $(REPEAT)); do \
	                        OUT=$$(mpirun -np $$np $$BIN $$n $$indiv $$INF 2>/dev/null); \
	                        GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
	                        GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
	                        GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
	                        AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
	                        AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
	                        AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
	                        echo "mpi,$$opt,$$np,$$n,$$indiv,$$pct,$$INF,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
	                        sleep 1; \
	                    done; \
	                done; \
	            done; \
	        done; \
	    done; \
	done

bench: run_bench_mpi run_bench_omp run_bench_seq


########################################
# CLEAN
########################################
clean:
	rm -f include/sources/*.o
	rm -f src/seq/*.o
	rm -f src/omp/*.o
	rm -f src/mpi/*.o

mrproper: clean
	rm -f $(SEQ_BINS) $(OMP_BINS) $(MPI_BINS)
