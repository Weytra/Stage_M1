# ============================
#   COMPILATEURS
# ============================
CC = gcc
MPICC = mpicc
MPI_FLAGS=--oversubscribe --mca btl vader,self --mca btl_tcp_if_exclude lo,docker,virbr0,enp0s3
SEQ_FLAGS = -march=native -funroll-loops

#Flag a ajouter a seq_flag pour les teste sur la vectorisation !
#-ftree-vectorize
#-fno-tree-vectorize


# ============================
#   OPTIMISATIONS
# ============================
OPTS = O0 
#O1 O2 O3 Ofast

# ============================
#   BIBLIOTEQUE
# ============================
LIBS = -lm

# ============================
#   INCLUDE
# ============================
INC = -Isrc/include/headers
COMMON_SRC = $(wildcard src/include/sources/*.c)

# ============================
#   DOSSIERS
# ============================
SRC_SEQ = src/seq
SRC_PAR = src/parallele

BIN_SEQ = bin/seq
BIN_PAR = bin/parallele

# ============================
#   LISTE DES SOURCES
# ============================
SEQ_SRCS = $(wildcard $(SRC_SEQ)/*.c)
PAR_SRCS = $(wildcard $(SRC_PAR)/*.c)

# ============================
#   NOMS DES PROGRAMMES
# ============================
SEQ_NAMES = $(notdir $(basename $(SEQ_SRCS)))
PAR_NAMES = $(notdir $(basename $(PAR_SRCS)))


# ============================
#   REGLES PRINCIPALES
# ============================
all: build_dirs build_result_dirs compile_seq compile_par

# ============================
#   CREATION DES DOSSIERS
# ============================
build_dirs:
	for name in $(SEQ_NAMES); do \
	    for opt in $(OPTS); do \
	        mkdir -p $(BIN_SEQ)/$$name/$$opt; \
	    done; \
	done
	for name in $(PAR_NAMES); do \
	    for opt in $(OPTS); do \
	        mkdir -p $(BIN_PAR)/$$name/$$opt; \
	    done; \
	done

# ===================================
#   CREATION DES DOSSIERS RESULTAT
# ===================================
build_result_dirs:
	for name in $(SEQ_NAMES); do \
	    for opt in $(OPTS); do \
	        mkdir -p resultat/seq/$$name/$$opt; \
	    done; \
	done
	for name in $(PAR_NAMES); do \
	    for opt in $(OPTS); do \
	        mkdir -p resultat/parallele/$$name/$$opt; \
	    done; \
	done


# ============================
#   COMPILATION SEQUENTIELLE
# ============================
compile_seq:
	for src in $(SEQ_SRCS); do \
	    name=$$(basename $$src .c); \
	    for opt in $(OPTS); do \
	        mkdir -p $(BIN_SEQ)/$$name/$$opt; \
	        $(CC) -$$opt $(SEQ_FLAGS) $(INC) $$src $(COMMON_SRC) -o $(BIN_SEQ)/$$name/$$opt/$$name; \
	        echo "Compiled $$name with -$$opt"; \
	    done; \
	done


# ============================
#   COMPILATION PARALLELE
# ============================
compile_par:
	for src in $(PAR_SRCS); do \
	    name=$$(basename $$src .c); \
	    for opt in $(OPTS); do \
	        $(MPICC) -$$opt $$src $(LIBS) $(INC) $(COMMON_SRC) -o $(BIN_PAR)/$$name/$$opt/$$name; \
	        echo "Compiled MPI $$name with -$$opt"; \
	    done; \
	done

# ============================
#   BENCHMARK
# ============================

SIZES = 64 128 256 512 1024 2048 4096

NPS = 2 4 8 12 16

GRID_ALGOS = cannon_2 fox_2 fox_3 fox_cop SUMMA_

REPEAT = 5

# ============================
#   BENCHMARK PARALLELE
# ============================

run_all_paral:

	for name in $(PAR_NAMES); do \
	  for opt in $(OPTS); do \
\
	    CSV=resultat/parallele/$$name/$$opt/result.csv; \
	    TXT=resultat/parallele/$$name/$$opt/result.txt; \
\
	    mkdir -p resultat/parallele/$$name/$$opt; \
\
	    echo "exec,opt,np,size,global_time,global_energy,global_power,algo_time,algo_energy,algo_power" > $$CSV; \
	    > $$TXT; \
\
	    if [ ! -x $(BIN_PAR)/$$name/$$opt/$$name ]; then \
	        echo "Skipping $$name"; \
	        continue; \
	    fi; \
\
	    if echo "$(GRID_ALGOS)" | grep -wq "$$name"; then \
	        NPLIST="4 9 16"; \
	    else \
	        NPLIST="$(NPS)"; \
	    fi; \
\
	    for np in $$NPLIST; do \
	      for size in $(SIZES); do \
\
	        echo "Running $$name -$$opt NP=$$np SIZE=$$size"; \
\
	        for r in $$(seq 1 $(REPEAT)); do \
\
	          OUT=$$(mpiexec $(MPI_FLAGS) \
	                -n $$np \
	                $(BIN_PAR)/$$name/$$opt/$$name \
	                1 $$size 8 TMP 2>/dev/null); \
\
	          GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
	          GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
	          GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
\
	          AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
	          AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
	          AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
\
	          echo "$$name,$$opt,$$np,$$size,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
\
	          echo "EXEC=$$name OPT=$$opt NP=$$np SIZE=$$size REP=$$r GT=$$GT AT=$$AT" >> $$TXT; \
\
	          sleep 1; \
\
	        done; \
\
	      done; \
	    done; \
\
	  done; \
	done

#/!\ si le block est plus gros que la taille de la matrice cela n'auras juste pas d'effet
SIZES = 64 128 256 512 1024 2048 4096 8192

BLOCK = 8 16 32 64 128 256

run_all_seq:
	@for name in $(SEQ_NAMES); do \
	  for opt in $(OPTS); do \
\
	    DIR=resultat/seq/$$name/$$opt; \
	    CSV=$$DIR/result.csv; \
	    TXT=$$DIR/result.txt; \
\
	    mkdir -p $$DIR; \
\
	    echo "exec,opt,block,size,global_time,global_energy,global_power,algo_time,algo_energy,algo_power" > $$CSV; \
	    > $$TXT; \
\
	    BIN=$(BIN_SEQ)/$$name/$$opt/$$name; \
	    if [ ! -x $$BIN ]; then \
	        echo "Skipping $$name (binary not found)"; \
	        continue; \
	    fi; \
\
	    for block in $(BLOCK); do \
	      for size in $(SIZES); do \
\
	        echo "Running $$name -$$opt BLOCK=$$block SIZE=$$size"; \
\
	        for r in $$(seq 1 $(REPEAT)); do \
\
	          OUT=$$($$BIN $$size $$block 2>/dev/null); \
\
	          GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
	          GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
	          GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
\
	          AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
	          AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
	          AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
\
	          echo "$$name,$$opt,$$block,$$size,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
\
	          echo "EXEC=$$name OPT=$$opt BLOCK=$$block SIZE=$$size REP=$$r GT=$$GT AT=$$AT" >> $$TXT; \
\
	          sleep 1; \
\
	        done; \
\
	      done; \
	    done; \
\
	  done; \
	done




# ============================
#   RESULTATS GLOBAUX
# ============================

global_results:

	for name in $(PAR_NAMES); do \
	    outdir=resultat/parallele/$$name; \
	    outfile=$$outdir/resultat_global.csv; \
\
	    echo "exec,opt,np,size,global_time,global_energy,global_power,algo_time,algo_energy,algo_power" > $$outfile; \
\
	    for optdir in $$outdir/*; do \
	        csv=$$optdir/result.csv; \
	        if [ -f $$csv ]; then \
	            tail -n +2 $$csv >> $$outfile; \
	        fi; \
	    done; \
\
	    echo "Global results written to $$outfile"; \
	done; \
\
	topfile=resultat/parallele/resultat_global.csv; \
\
	echo "exec,opt,np,size,global_time,global_energy,global_power,algo_time,algo_energy,algo_power" > $$topfile; \
\
	for name in $(PAR_NAMES); do \
	    file=resultat/parallele/$$name/resultat_global.csv; \
	    if [ -f $$file ]; then \
	        tail -n +2 $$file >> $$topfile; \
	    fi; \
	done; \
\
	echo "Full global results written to $$topfile"



# ============================
#   BENCHMARK UNIQUE
# ============================
#
# make run_bench EXEC=cannon_2 OPT=O3 NP=4
#

run_bench:

	if [ -z "$(EXEC)" ] || [ -z "$(OPT)" ] || [ -z "$(NP)" ]; then \
	    echo "Usage : make run_bench EXEC=name OPT=O3 NP=4"; \
	    exit 1; \
	fi; \
\
	for size in $(SIZES); do \
	    echo ""; \
	    echo "===== SIZE $$size ====="; \
\
	    mpiexec $(MPI_FLAGS) \
	        -n $(NP) \
	        $(BIN_PAR)/$(EXEC)/$(OPT)/$(EXEC) \
	        1 $$size 8 TMP; \
	done

bench: run_all_paral global_results

run:

	if [ -z "$(EXEC)" ] || [ -z "$(OPT)" ] || [ -z "$(NP)" ]|| [ -z "$(SIZE)" ]; then \
	    echo "Usage : make run_bench EXEC=name OPT=O3 NP=4"; \
	    exit 1; \
	fi; \
\
	
	mpiexec $(MPI_FLAGS) \
		-n $(NP) \
		$(BIN_PAR)/$(EXEC)/$(OPT)/$(EXEC) \
		1 $(SIZE) 8 TMP; \
	

# ============================
#   CLEAN	
# ============================
clean:
	rm -rf bin/*

