CC = gcc
CFLAGS = -std=c99 -lm -D_POSIX_C_SOURCE=200809L
LDFLAGS = -pthread

all: integral_mc_shm integral_mc_shm_sem integral_mc_seq

integral_mc_shm: integral_mc_shm.c
	$(CC) -o integral_mc_shm integral_mc_shm.c $(CFLAGS) $(LDFLAGS)

integral_mc_shm_sem: integral_mc_shm_sem.c
	$(CC) -o integral_mc_shm_sem integral_mc_shm_sem.c $(CFLAGS) $(LDFLAGS)

integral_mc_seq: integral_mc_seq.c
	$(CC) -o integral_mc_seq integral_mc_seq.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f integral_mc_shm integral_mc_shm_sem integral_mc_seq
