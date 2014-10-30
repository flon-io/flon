
#CFLAGS=-I../src -g -Wall -O3 -lrt
CFLAGS=-I../src -g -Wall -lrt
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
	rm -f [cdxnil]s
	rm -f n_helpers.[ch]

n_helpers.o: n_helpers.c

DEPS=flutil.o gajeta.o aabro.o djan.o mnemo.o fl_common.o fl_ids.o
DDEPS=$(DEPS) fl_dispatcher.o
XDEPS=$(DEPS) fl_executor.o fl_exe_nodes.o fl_exe_instructions.o
IDEPS=$(DEPS) fl_invoker.o
NDEPS=$(DEPS) n_helpers.o

cs.c: ../spec/c_*_spec.c
	$(RODZO) ../spec/c_*_spec.c -o cs.c
ds.c: ../spec/d_*_spec.c
	$(RODZO) ../spec/d_*_spec.c -o ds.c
xs.c: ../spec/x_*_spec.c
	$(RODZO) ../spec/x_*_spec.c -o xs.c
is.c: ../spec/i_*_spec.c
	$(RODZO) ../spec/i_*_spec.c -o is.c
ns.c: ../spec/n/*_spec.c
	$(RODZO) ../spec/n/*_spec.c -o ns.c
	cp ../spec/n/n_helpers.[ch] .

cs: $(DEPS)
ds: $(DDEPS)
xs: $(XDEPS)
is: $(IDEPS)
ns: $(NDEPS)

sc: cs
	@echo "\n--- sc --------------------------------------------------------\n"
	time ./cs
sd: ds invoker executor
	@echo "\n--- sd --------------------------------------------------------\n"
	time ./ds
sx: xs
	@echo "\n--- sx --------------------------------------------------------\n"
	time ./xs
si: is
	@echo "\n--- si --------------------------------------------------------\n"
	time ./is
sn: ns invoker executor dispatcher
	@echo "\n--- sn --------------------------------------------------------\n"
	time ./ns

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
vsn: ns
	valgrind --leak-check=full -v ./ns
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./xs

ssi: is
	strace -r ./is

flon-executor: $(XDEPS)
flon-invoker: $(IDEPS)

flon-dispatcher: $(DDEPS)
	$(CC) -I../src $(CFLAGS) ../src/flon-dispatcher.c $(DDEPS) -lev -o flon-dispatcher

executor: flon-executor
	mv flon-executor ../tst/bin/
invoker: flon-invoker
	mv flon-invoker ../tst/bin/
dispatcher: flon-dispatcher
	mv flon-dispatcher ../tst/bin/

spec: sc sd sx si sn
s: spec
vspec: vsc vsd vsx vsi vsn
vs: vspec

.PHONY: clean

