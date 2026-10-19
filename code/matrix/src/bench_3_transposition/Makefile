# ============================
#   COMPILATEURS
# ============================
CC = gcc
MPICC = mpicc
MPI_FLAGS = --oversubscribe --mca btl vader,self --mca btl_tcp_if_exclude lo,docker,virbr0,enp0s3
SEQ_FLAGS = -O3 -march=native -funroll-loops -ftree-vectorize
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
AVX_TR_SRC = to_test/avx/mult_mat_seq_vecto_tr.c
MPI_SRC = to_test/mpi/SUMMA_.c
MPI_AVX_SRC = to_test/mpi_avx/SUMMA_vect.c

# ============================
#   BINAIRES
# ============================
SEQ_BIN = to_test/seq/matmul_seq
SEQ_BIN_PURE = to_test/seq/seq_pure
SEQ_BIN_AUTO_VECT = to_test/seq/seq_auto_vect

AVX_TR_BIN = to_test/avx/matmul_seq_avx_tr
MPI_BIN = to_test/mpi/summa_tr
MPI_AVX_BIN = to_test/mpi_avx/summa_vect_tr

# ============================
#   PARAMÈTRES BENCHMARK
# ============================
SIZES = 256 512 1024 2048
PROCS = 4 9 16
REPEAT = 5
BLOCK = 8 16 32 64

# ============================
#   RÈGLES PRINCIPALES
# ============================
all: build run

build: $(SEQ_BIN) $(MPI_AVX_BIN) $(AVX_TR_BIN) $(MPI_BIN) $(MPI_AVX_BIN)

# ============================
#   COMPILATION
# ============================

$(SEQ_BIN): $(SEQ_SRC) $(COMMON_SRC)
	$(CC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(AVX_TR_BIN): $(AVX_TR_SRC) $(COMMON_SRC)
	$(CC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_BIN): $(MPI_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)

$(MPI_AVX_BIN): $(MPI_AVX_SRC) $(COMMON_SRC)
	$(MPICC) $(SEQ_FLAGS) $(INC) $^ -o $@ $(LDFLAGS)


# ============================
#   BENCHMARKS
# ============================

run: results_dir run_seq run_seq_blk run_avx_tr run_mpi run_mpi_avx
run_2:run_mpi_avx_blk run_seq_pure run_seq_auto_vect

results_dir:
	mkdir -p results

# ============================
#   BENCH SEQ
# ============================

run_seq:
	CSV=results/seq.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		echo "=== SEQ $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$($(SEQ_BIN) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "seq,$$n,1,1,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done

# ============================
#   BENCH SEQ BLK
# ============================

run_seq_blk:
	CSV=results/seq_blk_avx_tr.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for blk in $(BLOCK); do\
			echo "=== SEQ $$n $$blk ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$($(SEQ_BIN) 1 $$n $$blk 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "seq_blk_avx_tr,$$n,$$blk,1,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done


# ============================
#   BENCH AVX TR
# ============================

run_avx_tr:
	CSV=results/seq_avx_tr.csv; \
	echo "exec,size,block,no,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		echo "=== AVX_TR $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$($(AVX_TR_BIN) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "seq_avx_tr,$$n,1,1,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done

# ============================
#   BENCH MPI
# ============================

run_mpi:
	CSV=results/summa_tr.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for p in $(PROCS); do \
			echo "=== MPI $$p $$n ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(mpiexec $(MPI_FLAGS) -np $$p $(MPI_BIN) 1 $$n 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "summa_tr,$$n,1,$$p,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done

# ============================
#   BENCH MPI + AVX
# ============================

run_mpi_avx:
	CSV=results/summa_vect_man_tr.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for p in $(PROCS); do \
			echo "=== MPI_AVX $$p $$n ==="; \
			for r in $$(seq 1 $(REPEAT)); do \
				OUT=$$(mpiexec $(MPI_FLAGS) -np $$p $(MPI_AVX_BIN) 1 $$n 2>/dev/null); \
				GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
				GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
				GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
				AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
				AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
				AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
				echo "summa_vect_tr,$$n,1,$$np,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
				sleep 1; \
			done; \
		done; \
	done



run_mpi_avx_blk:
	CSV=results/mpi_avx_blk_tr.csv; \
	echo "exec,size,block,np,threads,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		for p in $(PROCS); do \
			for blk in $(BLOCK); do \
				echo "=== MPI_AVX $$p $$n $$blk ==="; \
				for r in $$(seq 1 $(REPEAT)); do \
					OUT=$$(mpiexec $(MPI_FLAGS) -np $$p $(MPI_AVX_BIN) 1 $$n $$blk 2>/dev/null); \
					GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
					GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
					GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
					AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
					AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
					AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
					echo "mpi_avx_blk_tr,$$n,$$blk,$$p,1,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
					sleep 1; \
				done; \
			done; \
		done; \
	done

run_seq_pure:
	CSV=results/seq_pure.csv; \
	echo "exec,size,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		echo "=== SEQ $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$($(SEQ_BIN_PURE) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "seq_pure,$$n,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done



run_seq_auto_vect:
	CSV=results/seq_auto_vect.csv; \
	echo "exec,size,GT,GE,GP,AT,AE,AP" > $$CSV; \
	for n in $(SIZES); do \
		echo "=== SEQ $$n ==="; \
		for r in $$(seq 1 $(REPEAT)); do \
			OUT=$$($(SEQ_BIN_AUTO_VECT) 1 $$n 2>/dev/null); \
			GT=$$(echo "$$OUT" | grep GLOBAL_TIME | cut -d= -f2); \
			GE=$$(echo "$$OUT" | grep GLOBAL_ENERGY | cut -d= -f2); \
			GP=$$(echo "$$OUT" | grep GLOBAL_POWER | cut -d= -f2); \
			AT=$$(echo "$$OUT" | grep ALGO_TIME | cut -d= -f2); \
			AE=$$(echo "$$OUT" | grep ALGO_ENERGY | cut -d= -f2); \
			AP=$$(echo "$$OUT" | grep ALGO_POWER | cut -d= -f2); \
			echo "seq_auto_vect,$$n,$$GT,$$GE,$$GP,$$AT,$$AE,$$AP" >> $$CSV; \
			sleep 1; \
		done; \
	done


# ============================
#   CLEAN
# ============================
clean:
	rm -f $(SEQ_BIN) $(AVX_BIN) $(AVX_TR_BIN) $(MPI_BIN) $(MPI_AVX_BIN)

clean_results:
	rm -rf results/*
