
#CFLAGS=-I../src -g -Wall -O3
CFLAGS=-I../src -Wall -O3
LDLIBS=
CC=c99
VPATH=../src

m: mirror
	cp mirror ../bin/

clean:
	rm -f *.o
	rm -f *.c
	rm -f $(NAME)
	rm -f ../bin/$(NAME)

.PHONY: clean

