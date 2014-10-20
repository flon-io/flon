
#CFLAGS=-I../src -g -Wall -O3
CFLAGS=-I../src -g -Wall
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
	rm -f [cdxi]s

DEPS=flutil.o gajeta.o aabro.o djan.o mnemo.o fl_common.o fl_ids.o
DDEPS=$(DEPS) fl_dispatcher.o
XDEPS=$(DEPS) fl_executor.o
IDEPS=$(DEPS) fl_invoker.o

cs.c: ../spec/c_*_spec.c
	$(RODZO) ../spec/c_*_spec.c -o cs.c
ds.c: ../spec/d_*_spec.c
	$(RODZO) ../spec/d_*_spec.c -o ds.c
xs.c: ../spec/x_*_spec.c
	$(RODZO) ../spec/x_*_spec.c -o xs.c
is.c: ../spec/i_*_spec.c
	$(RODZO) ../spec/i_*_spec.c -o is.c

cs: $(DEPS)
ds: $(DDEPS)
xs: $(XDEPS)
is: $(IDEPS)

sc: cs
	time ./cs
sd: ds invoker
	time ./ds
sx: xs
	time ./xs
si: is
	time ./is

vsc: cs
	valgrind --leak-check=full -v ./cs
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./cs
vsd: ds invoker
	valgrind --leak-check=full -v ./ds
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./cs
vsx: xs
	valgrind --leak-check=full -v ./xs
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./xs
vsi: is
	valgrind ./is # an invoker may lose as much mem as he wants...
	#valgrind --leak-check=full -v ./is
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./is

ssi: is
	strace -r ./is

flon-executor: $(XDEPS)
flon-invoker: $(IDEPS)

executor: flon-executor
	mv flon-executor ../tst/bin/
invoker: flon-invoker
	mv flon-invoker ../tst/bin/

spec:
	$(MAKE) sc
	#$(MAKE) sl
	$(MAKE) sd
	$(MAKE) sx
	$(MAKE) si

.PHONY: clean

