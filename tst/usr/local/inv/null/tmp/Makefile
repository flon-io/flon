
CFLAGS=-I../src -g -Wall -O3 -lrt
LDLIBS=
CC=c99
VPATH=../src

null: flutil.o flutim.o gajeta.o

n: null
	cp null ../bin/

clean:
	rm -f *.o
	rm -f *.c
	rm -f $(NAME)
	rm -f ../bin/$(NAME)

.PHONY: clean

