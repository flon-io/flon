
#CFLAGS=-I../src -g -Wall -O3 -lrt
CFLAGS=-I../src -g -Wall -lrt -lev
#CFLAGS=-I../src -g -Wall -lrt -lev -D_POSIX_C_SOURCE=200809L
LDLIBS=
CC=c99
VPATH=../src

VAL=valgrind --leak-check=full -v
#VAL=valgrind --leak-check=full -v --num-callers=50
#VAL=valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v

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
  fl_common.o fl_paths.o fl_ids.o fl_tools.o
#
DDEPS=$(DEPS) flu64.o fl_dispatcher.o
XDEPS=$(DEPS) dollar.o fl_executor.o fl_exe_nodes.o fl_exe_instructions.o
IDEPS=$(DEPS) dollar.o fl_invoker.o
#
LDEPS=$(DEPS) $(SHV_OS) $(TSI_OS) flu64.o fl_listener.o fl_lis_auth.o

feu_hlp:
	cp ../spec/feu_helpers.[ch] .

cs.c: ../spec/c/*_spec.c
	$(RODZO) ../spec/c/*_spec.c -o cs.c
ds.c: ../spec/d/*_spec.c feu_hlp
	$(RODZO) ../spec/d/*_spec.c -o ds.c
xs.c: ../spec/x/*_spec.c
	$(RODZO) ../spec/x/*_spec.c -o xs.c
is.c: ../spec/i/*_spec.c
	$(RODZO) ../spec/i/*_spec.c -o is.c
ls.c: ../spec/l/*_spec.c feu_hlp
	$(RODZO) ../spec/l/*_spec.c -o ls.c
	cp ../spec/l/l_helpers.[ch] .
ns.c: ../spec/n/*_spec.c feu_hlp
	$(RODZO) ../spec/n/*_spec.c -o ns.c
zs.c: ../spec/z/*_spec.c feu_hlp
	$(RODZO) ../spec/z/*_spec.c -o zs.c

cs: $(DEPS)
ds: $(DDEPS) feu_helpers.o
xs: $(XDEPS)
is: $(IDEPS)
ls: $(LDEPS) fl_dispatcher.o l_helpers.o
ns: $(DEPS) feu_helpers.o
zs: $(DEPS) feu_helpers.o

s: cs ds xs is ls ns zs
warn:
	$(MAKE) clean cs ds xs is ls ns zs  2>&1 | . ./spec_filter.sh

sc: cs
	@echo "\n--- make sc --common-------------------------------------------\n"
	time ./cs
sd: ds invoker executor
	@echo "\n--- make sd --dispatcher---------------------------------------\n"
	time ./ds
sx: xs
	@echo "\n--- make sx --executor-----------------------------------------\n"
	time ./xs
si: is
	@echo "\n--- make si --invoker------------------------------------------\n"
	time ./is
sl: ls invoker executor dispatcher
	@echo "\n--- make sl --listener-----------------------------------------\n"
	time ./ls
sn: ns invoker executor dispatcher
	@echo "\n--- make sn --instructions-------------------------------------\n"
	time ./ns
sz: zs invoker executor dispatcher
	@echo "\n--- make sz --a-to-z-------------------------------------------\n"
	time ./zs

sum: cs ds is ls ns xs zs invoker executor dispatcher
	$(MAKE) -C .. ctst 2>&1 > /dev/null
	@echo "\n--- make sc --common-------------------------------------------"
	@./cs 2>&1 | . ./spec_filter.sh
	@echo "\n--- make sd --dispatcher---------------------------------------"
	@./ds 2>&1 | . ./spec_filter.sh
	@echo "\n--- make sx --executor-----------------------------------------"
	@./xs 2>&1 | . ./spec_filter.sh
	@echo "\n--- make si --invoker------------------------------------------"
	@./is 2>&1 | . ./spec_filter.sh
	@echo "\n--- make sl --listener-----------------------------------------"
	@./ls 2>&1 | . ./spec_filter.sh
	@echo "\n--- make sn --instructions-------------------------------------"
	@./ns 2>&1 | . ./spec_filter.sh
	@echo "\n--- make sz --a-to-z-------------------------------------------"
	@./zs 2>&1 | . ./spec_filter.sh
	@echo ""

vsum: cs ds is ls ns xs zs invoker executor dispatcher
	$(MAKE) -C .. ctst 2>&1 > /dev/null
	@echo "\n--- make vsc --common------------------------------------------"
	$(VAL) ./cs 2>&1 | . ./spec_filter.sh
	@echo "\n--- make vsd --dispatcher--------------------------------------"
	$(VAL) ./ds 2>&1 | . ./spec_filter.sh
	@echo "\n--- make vsx --executor----------------------------------------"
	$(VAL) ./xs 2>&1 | . ./spec_filter.sh
	@echo "\n--- make vsi --invoker-----------------------------------------"
	$(VAL) ./is 2>&1 | . ./spec_filter.sh
	@echo "\n--- make vsl --listener----------------------------------------"
	$(VAL) ./ls 2>&1 | . ./spec_filter.sh
	@echo "\n--- make vsn --instructions------------------------------------"
	$(VAL) ./ns 2>&1 | . ./spec_filter.sh
	@echo "\n--- make vsz --a-to-z------------------------------------------"
	$(VAL) ./zs 2>&1 | . ./spec_filter.sh
	@echo ""

vsc: cs
	$(VAL) ./cs
vsd: ds executor invoker
	$(VAL) ./ds
vsx: xs
	$(VAL) --num-callers=50 ./xs
vsi: is
	$(VAL) ./is
vsl: ls invoker executor dispatcher
	$(VAL) --num-callers=50 ./ls
vsn: ns invoker executor dispatcher
	$(VAL) ./ns
vsz: zs invoker executor dispatcher
	$(VAL) ./zs

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

spec:
	time -p $(MAKE) sum
vspec:
	time -p $(MAKE) vsum

.PHONY: clean

