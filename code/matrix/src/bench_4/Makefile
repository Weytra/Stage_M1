# ============================
#   COMPILATEURS
# ============================
CC = gcc
MPICC = mpicc
OMPFLAG = -fopenmp
MPI_FLAGS = --oversubscribe --mca btl vader,self --mca btl_tcp_if_exclude lo,docker,virbr0,enp0s3

SEQ_FLAGS = -O3 -march=native -funroll-loops -ftree-vectorize
SEQ_FLAGS_F = -Ofast -march=native -funroll-loops -ftree-vectorize

LDFLAGS = -lm

# ============================
#   INCLUDE + SOURCES COMMUNS
# ============================
INC = -I../include/headers
COMMON_SRC = $(wildcard ../include/sources/*.c)

# ============================
#   SOURCES À TESTER
# ============================
SEQ_SRC = to_test/seq/mult_mat_seq_blocking.c
OMP_SRC = to_test/omp/mult_mat_omp.c
MPI_SRC = to_test/mpi/SUMMA_.c
MPI_AVX_SRC = to_test/mpi_avx/SUMMA_vect.c
MPI_OMP_SRC = to_test/mpi_omp/SUMMA_.c
MPI_OMP_AVX_SRC = to_test/mpi_omp_vect/SUMMA_vect.c

# ============================
#   BINAIRES
# ============================
SEQ_BIN = bin/seq_blk
OMP_BIN = bin/omp
MPI_BIN = bin/mpi
MPI_AVX_BIN = bin/mpi_avx
MPI_OMP_BIN = bin/mpi_omp
MPI_OMP_AVX_BIN = bin/mpi_omp_avx
MPI_OMP_AVX_BIN_Ofast = bin/mpi_omp_avx_Ofast

# ============================
#   PARAMÈTRES BENCHMARK
# ============================
SIZES = 256 512 1024 2048 4096 8192
SIZES_SEQ_BLK = 256 512 1024 2048 4096
BLOCKS = 8 16 32 64
PROCS = 4 9 16
OMP_THREADS = 16
OMP_THREADS_MPI = 4
REPEAT = 5

# ============================
#   RÈGLES PRINCIPALES
# ============================
all: build run

build: dirs $(SEQ_BIN) $(OMP_BIN) $(MPI_BIN) $(MPI_AVX_BIN) $(MPI_OMP_BIN) $(MPI_OMP_AVX_BIN) $(MPI_OMP_AVX_BIN_Ofast)

dirs:
	mkdir -p bin results

# ============================
#   COMPILATION
# ============================

$(SEQ_BIN): $(SEQ_SRC) $(COMMON_SRC)
	$(CC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(OMP_BIN): $(OMP_SRC) $(COMMON_SRC)
	$(CC) $(SEQ_FLAGS) $(OMPFLAG) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_BIN): $(MPI_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_AVX_BIN): $(MPI_AVX_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_OMP_BIN): $(MPI_OMP_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(OMPFLAG) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_OMP_AVX_BIN): $(MPI_OMP_AVX_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(OMPFLAG) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_OMP_AVX_BIN_Ofast): $(MPI_OMP_AVX_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS_F) $(OMPFLAG) $(INC) $^ -o $@ $(LDFLAGS)

# ============================
#   BENCHMARKS
# ============================

run: run_seq_blk run_omp run_mpi run_mpi_avx run_mpi_omp run_mpi_omp_avx
run_ult : run_mpi_omp_avx_blk run_mpi_omp_avx_blk_Of

# ============================
#   SEQ + BLOCKING
# ============================
#tester en taille 8192, environ 40 minutes avec repeat = 5
run_seq_blk:
	CSV=results/seq_blk.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES_SEQ_BLK); do \
		for blk in $(BLOCKS); do \
			echo "=== SEQ $$n $$blk ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$($(SEQ_BIN) 1 $$n $$blk 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "seq_blk,$$n,$$blk,1,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

# ============================
#   OMP (16 THREADS)
# ============================

run_omp:
	CSV=results/omp.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	export OMP_NUM_THREADS=$(OMP_THREADS); \
	for n in $(SIZES); do \
		echo "=== OMP $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$($(OMP_BIN) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "omp,$$n,1,1,$$OMP_THREADS,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done

# ============================
#   MPI (4, 9, 16)
# ============================

run_mpi:
	CSV=results/mpi.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for p in $(PROCS); do \
			echo "=== MPI $$n np = $$p ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(mpiexec $(MPI_FLAGS) -np $$p $(MPI_BIN) 1 $$n 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "mpi,$$n,1,$$p,threads,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

# ============================
#   MPI + AVX
# ============================

run_mpi_avx:
	CSV=results/mpi_avx.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for p in $(PROCS); do \
			echo "=== MPI avx $$n np = $$p ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(mpiexec $(MPI_FLAGS) -np $$p $(MPI_AVX_BIN) 1 $$n 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "mpi_avx,$$n,1,$$p,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

# ============================
#   MPI + OMP (4 MPI × 4 OMP)
# ============================

run_mpi_omp:
	CSV=results/mpi_omp.csv; \
	echo "exec,size,,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	export OMP_NUM_THREADS=$(OMP_THREADS_MPI); \
	for n in $(SIZES); do \
		echo "=== MPI omp $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$(mpiexec $(MPI_FLAGS) -np 4 $(MPI_OMP_BIN) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "mpi_omp,$$n,1,4,$$OMP_THREADS,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done

# ============================
#   MPI + OMP + AVX
# ============================

run_mpi_omp_avx:
	CSV=results/mpi_omp_avx.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	export OMP_NUM_THREADS=$(OMP_THREADS_MPI); \
	for n in $(SIZES); do \
		echo "=== MPI_omp_avx avx $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$(mpiexec $(MPI_FLAGS) -np 4 $(MPI_OMP_AVX_BIN) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "mpi_omp_avx,$$n,1,4,$$OMP_THREADS,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done

# ============================
#   MPI + OMP + AVX + BLK
# ============================

run_mpi_omp_avx_blk:
	CSV=results/mpi_omp_avx_blk_tr.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	export OMP_NUM_THREADS=$(OMP_THREADS_MPI); \
	for n in $(SIZES); do \
		for blk in $(BLOCKS); do \
			echo "=== MPI_omp_avx avx $$n $$blk ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(mpiexec $(MPI_FLAGS) -np 4 $(MPI_OMP_AVX_BIN) 1 $$n $$blk 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "mpi_omp_avx_blk,$$n,$$blk,4,$$OMP_THREADS,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

# ============================
#   MPI + OMP + AVX + BLK + Ofast
# ============================

run_mpi_omp_avx_blk_Of:
	CSV=results/mpi_omp_avx_blk_tr_Of.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	export OMP_NUM_THREADS=$(OMP_THREADS_MPI); \
	for n in $(SIZES); do \
		for blk in $(BLOCKS); do \
			echo "=== MPI_omp_avx avx $$n $$blk ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(mpiexec $(MPI_FLAGS) -np 4 $(MPI_OMP_AVX_BIN_Ofast) 1 $$n $$blk 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "mpi_omp_avx_blk_Of,$$n,$$blk,4,$$OMP_THREADS,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

# ============================
#   CLEAN
# ============================
clean:
	rm -rf bin/*

clean_results:
	rm -rf results/*
