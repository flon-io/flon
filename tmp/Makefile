
CFLAGS=-I../src -g -Wall -O3
LDLIBS=
CC=c99
VPATH=../src

RODZO=$(shell which rodzo)
ifeq ($(RODZO),)
  RODZO=../../rodzo/bin/rodzo
endif

#NAMES=$(patsubst $(VPATH)/%.c, ./%.o, $(wildcard $(VPATH)/*.c))
#  # since wildcard doesn't care about VPATH...
#
#s.c: ../spec/*_spec.c
#	$(RODZO) ../spec -o s.c
#
#s: $(NAMES)
#
#spec: s
#	time ./s
#
#vspec: s
#	valgrind --leak-check=full -v ./s
#	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./s
#
#dspec: s
#	gdb ./s

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.c
	#rm -f s

XDEPS=flutil.o aabro.o djan.o

xs.c: ../spec/x_*_spec.c
	$(RODZO) ../spec/x_*_spec.c -o xs.c

xs: $(XDEPS)

specx: xs
	time ./xs

flon-executor: $(XDEPS)

executor: flon-executor
	mv flon-executor ../bin

spec:
	$(MAKE) specx
	#$(MAKE) specd
	#$(MAKE) specl

.PHONY: spec specx clean

