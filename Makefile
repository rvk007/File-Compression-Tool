CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyuenc

nyuenc: nyuenc.o encode_file.o utility.o
nyuenc.o: encode_file.c utility.h
encode_file.o: encode_file.c encode_file.h
utility.o : utility.c utility.h

.PHONY: clean
clean:
	rm -f *.o nyuenc
	