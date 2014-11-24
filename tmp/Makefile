
#CFLAGS=-I../src -g -Wall -O3 -lrt
CFLAGS=-I../src -g -Wall -lrt -lev
LDLIBS=
CC=c99
VPATH=../src

RODZO=$(shell which rodzo)
ifeq ($(RODZO),)
  RODZO=../../rodzo/bin/rodzo
endif

SHV_OS=\
  $(patsubst $(VPATH)/%.c, ./%.o, $(wildcard $(VPATH)/sh*.c))
TSI_OS=\
  $(patsubst $(VPATH)/%.c, ./%.o, $(wildcard $(VPATH)/crypt_*.c)) tsifro.o

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.c
	rm -f [cdxnil]s
	rm -f feu_helpers.[ch]
	rm -f l_helpers.[ch]

feu_helpers.o: feu_helpers.c
l_helpers.o: l_helpers.c

DEPS=\
  flutil.o flutim.o \
  gajeta.o aabro.o djan.o mnemo.o \
  fl_common.o fl_ids.o fl_tools.o
#
DDEPS=$(DEPS) fl_dispatcher.o
XDEPS=$(DEPS) fl_executor.o fl_exe_nodes.o fl_exe_instructions.o
IDEPS=$(DEPS) dollar.o fl_invoker.o
#
LDEPS=$(DEPS) $(SHV_OS) $(TSI_OS) flu64.o fl_listener.o fl_lis_auth.o

cs.c: ../spec/c_*_spec.c
	$(RODZO) ../spec/c_*_spec.c -o cs.c
ds.c: ../spec/d_*_spec.c
	$(RODZO) ../spec/d_*_spec.c -o ds.c
xs.c: ../spec/x_*_spec.c
	$(RODZO) ../spec/x_*_spec.c -o xs.c
is.c: ../spec/i/*_spec.c
	$(RODZO) ../spec/i/*_spec.c -o is.c
ls.c: ../spec/l/*_spec.c
	$(RODZO) ../spec/l/*_spec.c -o ls.c
	cp ../spec/l/l_helpers.[ch] .
ns.c: ../spec/n/*_spec.c
	$(RODZO) ../spec/n/*_spec.c -o ns.c
	cp ../spec/feu_helpers.[ch] .
zs.c: ../spec/z/*_spec.c
	$(RODZO) ../spec/z/*_spec.c -o zs.c
	cp ../spec/feu_helpers.[ch] .

cs: $(DEPS)
ds: $(DDEPS)
xs: $(XDEPS)
is: $(IDEPS)
ls: $(LDEPS) fl_dispatcher.o l_helpers.o
ns: $(DEPS) feu_helpers.o
zs: $(DEPS) feu_helpers.o

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
sl: ls invoker executor dispatcher
	@echo "\n--- sl --------------------------------------------------------\n"
	time ./ls
sn: ns invoker executor dispatcher
	@echo "\n--- sn --------------------------------------------------------\n"
	time ./ns
sz: zs invoker executor dispatcher
	@echo "\n--- sz --------------------------------------------------------\n"
	time ./zs

vsc: cs
	valgrind --leak-check=full -v ./cs
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./cs
vsd: ds executor invoker
	valgrind --leak-check=full -v ./ds
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./cs
vsx: xs
	valgrind --leak-check=full -v ./xs
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./xs
vsi: is
	#valgrind ./is # an invoker may lose as much mem as he wants...
	valgrind --leak-check=full -v ./is
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./is
vsl: ls
	#valgrind --leak-check=full -v ./ls
	valgrind --leak-check=full -v --num-callers=50 ./ls
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./xs
vsn: ns
	valgrind --leak-check=full -v ./ns
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./xs
vsz: zs
	valgrind --leak-check=full -v ./zs
	#valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v ./xs

ssi: is
	strace -r ./is
ssd: ds executor invoker
	strace -r ./ds

flon-executor: $(XDEPS)
flon-invoker: $(IDEPS)
#
flon-dispatcher: $(DDEPS)
	$(CC) -I../src $(CFLAGS) ../src/flon-dispatcher.c $(DDEPS) \
      -lev -o flon-dispatcher
flon-listener: $(LDEPS)
	$(CC) -I../src $(CFLAGS) ../src/flon-listener.c $(LDEPS) \
      -lev -o flon-listener
#
flon-scope: $(DEPS)
flon-flon: $(DEPS) $(TSI_OS) flu64.o

executor: flon-executor
	mv flon-executor ../tst/bin/
invoker: flon-invoker
	mv flon-invoker ../tst/bin/
dispatcher: flon-dispatcher
	mv flon-dispatcher ../tst/bin/
listener: flon-listener
	mv flon-listener ../tst/bin/
scope: flon-scope
	mv flon-scope ../tst/bin/
fflon: flon-flon
	mv flon-flon ../tst/bin/

spec: sc sd sx si sl sn sz
s: spec
vspec: vsc vsd vsx vsi vsl vsn vsz
vs: vspec

.PHONY: clean

