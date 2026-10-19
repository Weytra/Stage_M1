########################################
# COMPILATEURS
########################################
CC = gcc
MPICC = mpicc

########################################
# FLAGS
########################################
BASEFLAGS = -march=native -ftree-vectorize -funroll-loops -Iinclude/headers
OMPFLAGS  = -fopenmp
LDFLAGS   = -lm
MPI_FLAGS = --oversubscribe --mca btl vader,self --mca btl_tcp_if_exclude lo,docker,virbr0,enp0s3


########################################
# OPTIMISATIONS À TESTER
########################################
OPTS = O3

########################################
# PARAMÈTRES BENCHMARK
########################################
SIZES = 400 800 1200 2000
NB_INDIV = 20000 40000 80000 12000
INFECT_PCT = 30
NB_IT = 200
REPEAT = 5
MPI_PROCS = 4 8 16
OMP_THREADS = 16


########################################
# DOSSIERS BINAIRES
########################################
BIN_DIR = bin
SEQ_DIR = $(BIN_DIR)/seq
OMP_DIR = $(BIN_DIR)/omp
MPI_DIR = $(BIN_DIR)/mpi
HYBRID_DIR = $(BIN_DIR)/hybrid
PUR_DIR = $(BIN_DIR)/pure

$(shell mkdir -p $(SEQ_DIR) $(OMP_DIR) $(MPI_DIR) $(HYBRID_DIR))

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

# Appliquer aux fichiers HYBRID (MPI + OpenMP)
$(foreach opt,$(OPTS),$(eval $(call COMPILE_OPT_MPI,src/hybrid/infect_hybrid,$(opt))))

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

# HYBRID (MPI + OpenMP)
HYBRID_BINS = $(foreach opt,$(OPTS),$(HYBRID_DIR)/hybrid_exec_$(opt))

$(HYBRID_DIR)/hybrid_exec_%: src/hybrid/infect_hybrid_%.o \
	include/sources/fonctions_mpi_%.o \
	include/sources/Fonctions_%.o \
	include/sources/energy_monitor_%.o \
	include/sources/gest_arg_%.o
	$(MPICC) -$* $(BASEFLAGS) $(OMPFLAGS) $^ $(LDFLAGS) -o $@

########################################
# VERSION SEQUENTIELLE NAIVES 
########################################


########################################
# RÈGLE PRINCIPALE
########################################
all: $(SEQ_BINS) $(OMP_BINS) $(MPI_BINS) $(HYBRID_BINS)
all2: $(SEQ_BINS) $(OMP_BINS) $(MPI_BINS) $(HYBRID_BINS)


########################################
# BENCHMARK SEQ AVEC CALCUL POURCENTAGE
########################################

# pure 1 est en fait seq classique.
run_bench_seq:
	mkdir -p results; \
	CSV=results/infect_seq_C_NO_friendly.csv; \
	echo "exec,opt,size,nb_indiv,pct_infect,nb_infect,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for opt in $(OPTS); do \
		BIN=$(SEQ_DIR)/infect_$${opt}; \
		for n in $(SIZES); do \
			for indiv in $(NB_INDIV); do \
				for pct in $(INFECT_PCT); do \
					INF=$$(( indiv * pct / 100 )); \
					echo "=== SEQ $$opt $$n $$indiv $$pct% (INF=$$INF) ==="; \
					for r in $$(seq 1 $(REPEAT)); do \
						OUT=$$($$BIN $(NB_IT) $$n $$indiv $$INF 2>/dev/null); \
						GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
						GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
						GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
						AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
						AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
						AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
						echo "seq_C_NO_friendly$$opt,$$n,$$indiv,$$pct,$$INF,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
						sleep 1; \
					done; \
				done; \
			done; \
		done; \
	done


run_bench_omp:
	mkdir -p results; \
	CSV=results/infect_omp_C_NO_friendly..csv; \
	echo "exec,opt,size,nb_indiv,pct_infect,nb_infect,GT,GE,GP,AT,AE,AP" > $$CSV; \
	export OMP_NUM_THREADS=$(OMP_THREADS); \
	for opt in $(OPTS); do \
		BIN=$(OMP_DIR)/infect_omp_$${opt}; \
		for n in $(SIZES); do \
			for indiv in $(NB_INDIV); do \
				for pct in $(INFECT_PCT); do \
					INF=$$(( indiv * pct / 100 )); \
					echo "=== omp $$opt $$n $$indiv $$pct% (INF=$$INF) ==="; \
					for r in $$(seq 1 $(REPEAT)); do \
						OUT=$$($$BIN $(NB_IT) $$n $$indiv $$INF); \
						GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
						GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
						GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
						AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
						AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
						AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
						echo "omp_no_friendly,$$opt,$$n,$$indiv,$$pct,$$INF,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
						sleep 1; \
					done; \
				done; \
			done; \
		done; \
	done


run_bench_mpi:
	mkdir -p results; \
	CSV=results/infect_mpi_C_friendly..csv; \
	echo "exec,opt,np,size,nb_indiv,pct_infect,nb_infect,GT,GE,GP,AT,AE,AP" > $$CSV; \
	export OMP_NUM_THREADS=8; \
	for opt in $(OPTS); do \
	    for np in $(MPI_PROCS); do \
	        BIN=$(MPI_DIR)/mpi_exec_$${opt}; \
	        for n in $(SIZES); do \
	            for indiv in $(NB_INDIV); do \
	                for pct in $(INFECT_PCT); do \
	                    INF=$$(( indiv * pct / 100 )); \
	                    echo "=== MPI $$opt $$np proc $$n $$indiv $$pct% (INF=$$INF) ==="; \
	                    for r in $$(seq 1 $(REPEAT)); do \
	                        OUT=$$(mpirun $(MPI_FLAGS) -np $$np $$BIN $(NB_IT) $$n $$indiv $$INF 2>/dev/null); \
	                        GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
	                        GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
	                        GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
	                        AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
	                        AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
	                        AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
	                        echo "mpi_C_no_friendly,$$opt,$$np,$$n,$$indiv,$$pct,$$INF,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
	                        sleep 1; \
	                    done; \
	                done; \
	            done; \
	        done; \
	    done; \
	done

bench: run_bench_seq run_bench_omp run_bench_mpi run_bench_hybrid


########################################
# BENCHMARK HYBRID (MPI + OpenMP)
########################################

run_bench_hybrid:
	mkdir -p results; \
	CSV=results/infect_hybrid_MPI_OMP.csv; \
	echo "exec,opt,np,omp_threads,size,nb_indiv,pct_infect,nb_infect,GT,GE,GP,AT,AE,AP" > $$CSV; \
	\
	for opt in $(OPTS); do \
	    for np in $(MPI_PROCS); do \
	        BIN=$(HYBRID_DIR)/hybrid_exec_$${opt}; \
	        \
	        if [ $$np -eq 2 ]; then OMP_LIST="2 4 6 8"; fi; \
	        if [ $$np -eq 4 ]; then OMP_LIST="2 4"; fi; \
	        if [ $$np -eq 8 ]; then OMP_LIST="2"; fi; \
	        \
	        for omp in $$OMP_LIST; do \
	            export OMP_NUM_THREADS=$$omp; \
	            \
	            for n in $(SIZES); do \
	                for indiv in $(NB_INDIV); do \
	                    for pct in $(INFECT_PCT); do \
	                        INF=$$(( indiv * pct / 100 )); \
	                        echo "=== OPT $$opt MPI $$np OMP $$omp SIZE $$n INDIV $$indiv INF $$INF ==="; \
	                        \
	                        for r in $$(seq 1 $(REPEAT)); do \
	                            OUT=$$(mpirun $(MPI_FLAGS) -np $$np $$BIN $(NB_IT) $$n $$indiv $$INF 2>&1); \
	                            \
	                            GT=$$(echo "$$OUT" | awk -F= '/GLOBAL_TIME/ {print $$2; exit}'); \
	                            GE=$$(echo "$$OUT" | awk -F= '/GLOBAL_ENERGY/ {print $$2; exit}'); \
	                            GP=$$(echo "$$OUT" | awk -F= '/GLOBAL_POWER/ {print $$2; exit}'); \
	                            AT=$$(echo "$$OUT" | awk -F= '/ALGO_TIME/ {print $$2; exit}'); \
	                            AE=$$(echo "$$OUT" | awk -F= '/ALGO_ENERGY/ {print $$2; exit}'); \
	                            AP=$$(echo "$$OUT" | awk -F= '/ALGO_POWER/ {print $$2; exit}'); \
	                            \
	                            echo "hybrid,$$opt,$$np,$$omp,$$n,$$indiv,$$pct,$$INF,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
		                        sleep 1; \
	                        done; \
	                    done; \
	                done; \
	            done; \
	        done; \
	    done; \
	done

########################################
# CLEAN
########################################
clean:
	rm -f include/sources/*.o
	rm -f src/seq/*.o
	rm -f src/omp/*.o
	rm -f src/mpi/*.o
	rm -f src/hybrid/*.o

mrproper: clean
	rm -f $(SEQ_BINS) $(OMP_BINS) $(MPI_BINS) $(HYBRID_BINS)

restart: mrproper
	rm -f bin/mpi/*
	rm -f bin/seq/*
	rm -f bin/omp/*
	rm -f bin/hybrid/*
