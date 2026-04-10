## Part 1 Makefile — Sleeping Stylist with Semaphores

CC     = gcc
CFLAGS = -Wall -pthread

all: sleepingStylistSem

sleepingStylistSem: sleepingStylistSem.c
	$(CC) $(CFLAGS) -o sleepingStylistSem sleepingStylistSem.c

clean:
	rm -f sleepingStylistSem
