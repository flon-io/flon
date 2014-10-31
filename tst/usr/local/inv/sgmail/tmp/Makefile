
CFLAGS=-I../src -g -Wall -lrt -lcurl
LDLIBS=
CC=c99
VPATH=../src

sgmail: flutil.o bocla.o

m: sgmail
	cp sgmail ../bin/

clean:
	rm -f *.o
	rm -f *.c
	rm -f $(NAME)
	rm -f ../bin/$(NAME)

#.PHONY: spec vspec dspec clean
.PHONY: clean

