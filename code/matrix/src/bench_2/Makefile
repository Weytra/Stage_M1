# ============================
#   COMPILATEURS
# ============================
CC = gcc
MPICC = mpicc
MPI_RUN_FLAGS = --oversubscribe --mca btl vader,self --mca btl_tcp_if_exclude lo,docker,virbr0,enp0s3
SEQ_FLAGS = -O3 -fopenmp -march=native -funroll-loops -ftree-vectorize
LDFLAGS = -lm

# ============================
#   INCLUDE + SOURCES COMMUNS
# ============================
INC = -I../include/headers
COMMON_SRC = $(wildcard ../include/sources/*.c)

# ============================
#   SOURCES À TESTER
# ============================
SEQ_SRC = to_test/seq/mult_mat_seq_omp.c
AVX_OMP_SRC = to_test/omp_vecto/mult_mat_seq_omp_vect.c
MPI_OMP_SRC = to_test/mpi/SUMMA_.c
ULTIM_SRC = to_test/ultime/full_stack.c

# ============================
#   BINAIRES
# ============================

SEQ_BIN = to_test/seq/matmul_seq_omp
AVX_OMP_BIN = to_test/omp_vecto/matmul_avx_omp
MPI_OMP_BIN = to_test/mpi/matmul_mpi_omp
ULTIM_BIN = to_test/ultime/summa_fullstack

# ============================
#   PARAMÈTRES BENCHMARK
# ============================
SIZES = 256 512 1024 2048
PROCS = 4
REPEAT = 5
BLOCK = 8 16 32 64
OMP_THREADS = 16

# ============================
#   RÈGLES PRINCIPALES
# ============================
all: build run

build: $(SEQ_BIN) $(AVX_OMP_BIN) $(MPI_OMP_BIN) $(ULTIM_BIN)

# ============================
#   COMPILATION
# ============================

$(SEQ_BIN): $(SEQ_SRC) $(COMMON_SRC)
	$(CC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(AVX_OMP_BIN): $(AVX_OMP_SRC) $(COMMON_SRC)
	$(CC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_OMP_BIN): $(MPI_OMP_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(ULTIM_BIN): $(ULTIM_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)



# ============================
#   BENCHMARKS
# ============================

run: results_dir run_omp run_omp_blk run_avx_omp run_mpi_omp run_mpi_avx_fs

results_dir:
	mkdir -p results

# ============================
#   BENCH SEQ
# ============================

#on test ici seulement avec 16 threads
run_omp:
	CSV=results/omp.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		echo "=== SEQ $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$(export OMP_NUM_THREADS=$(OMP_THREADS); $(SEQ_BIN) 1 $$n 2>/dev/null); \
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

run_omp_blk:
	CSV=results/omp_block.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for blk in $(BLOCK); do\
			echo "=== SEQ $$n $$blk ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(export OMP_NUM_THREADS=$(OMP_THREADS); $(SEQ_BIN) 1 $$n $$blk 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "omp_block,$$n,$$blk,1,$$OMP_THREADS,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

# ============================
#   BENCH AVX
# ============================

run_avx_omp:
	CSV=results/avx_omp.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		echo "=== AVX $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$(export OMP_NUM_THREADS=$(OMP_THREADS); $(AVX_OMP_BIN) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "avx_omp,$$n,1,1,$$OMP_THREADS,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done

# ============================
#   BENCH MPI
# ============================

run_mpi_omp:
	CSV=results/mpi_omp.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for p in $(PROCS); do \
			echo "=== MPI $$p $$n ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(export OMP_NUM_THREADS=4; mpiexec $(MPI_RUN_FLAGS) -np $$p $(MPI_OMP_BIN) 1 $$n 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "mpi_omp,$$n,1,$$np,4,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

#ici l'oversubscription est essyer, avec np variable et threads = 4
run_mpi_avx_fs:
	CSV=results/mpi_avx_fs.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for p in $(PROCS); do \
			for blk in $(BLOCK); do \
				echo "=== MPI_AVX $$p $$n $$blk ==="; \
				for r in $$(seq 1 $(REPEAT)); do \
					OUT=$$(OMP_NUM_THREADS=4 mpiexec $(MPI_RUN_FLAGS) -np $$p $(ULTIM_BIN) 1 $$n $$blk 2>/dev/null); \
					GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
					GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
					GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
					AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
					AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
					AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
					echo "mpi_avx_fs,$$n,$$blk,$$p,4,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
					sleep 1; \
				done; \
			done; \
		done; \
	done



# ============================
#   CLEAN
# ============================
clean:
	rm -f \
	$(SEQ_BIN) \
	$(AVX_OMP_BIN) \
	$(MPI_OMP_BIN) \
	$(ULTIM_BIN)

clean_results:
	rm -rf results/*
