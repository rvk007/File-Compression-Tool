CC=gcc -pthread
CFLAGS=-g #-pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyuenc

nyuenc: nyuenc.o sequential_rle.o parallel_rle.o taskQueue.o
nyuenc.o: sequential_rle.c parallel_rle.c
parallel_rle.o: parallel_rle.c parallel_rle.h taskQueue.c
taskQueue.o: taskQueue.c taskQueue.h
sequential_rle.o: sequential_rle.c sequential_rle.h

.PHONY: clean
clean:
	rm -f *.o nyuenc

	