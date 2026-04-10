## Part 2 Makefile — Sleeping Stylist with Monitor

CC     = gcc
CFLAGS = -Wall -pthread

all: sleepingStylistMon

monitor.o: monitor.c monitor.h
	$(CC) $(CFLAGS) -c monitor.c

sleepingStylistMon: sleepingStylistMon.c monitor.o
	$(CC) $(CFLAGS) -o sleepingStylistMon sleepingStylistMon.c monitor.o

clean:
	rm -f sleepingStylistMon monitor.o
