
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

DEPS=flutil.o aabro.o djan.o fl_common.o
IDEPS=$(DEPS) fl_invoker.o
XDEPS=$(DEPS) fl_executor.o

cs.c: ../spec/c_*_spec.c
	$(RODZO) ../spec/c_*_spec.c -o cs.c
is.c: ../spec/i_*_spec.c
	$(RODZO) ../spec/i_*_spec.c -o is.c
xs.c: ../spec/x_*_spec.c
	$(RODZO) ../spec/x_*_spec.c -o xs.c

cs: $(DEPS)
is: $(IDEPS)
xs: $(XDEPS)

specc: cs
	time ./cs
speci: is
	time ./is
specx: xs
	time ./xs

#flon-executor: $(XDEPS)
#
#executor: flon-executor
#	mv flon-executor ../bin

spec:
	$(MAKE) specc
	$(MAKE) speci
	$(MAKE) specx
	#$(MAKE) specd
	#$(MAKE) specl

.PHONY: spec specx clean

