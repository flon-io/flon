
VPATH=src
#CFLAGS=-I../src -g -Wall -O3 -lrt
#CFLAGS=-I../src -g -Wall -lrt -lev -D_POSIX_C_SOURCE=200809L
CFLAGS=-I$(VPATH) -g -Wall -lrt -lev
LDLIBS=
CC=c99

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
	rm -f [cdxnilkz]s
	rm -f [cdxnilkz]s.c
	rm -f src/*.[ch]
	$(MAKE) _tmpsrc

DEPS= \
  flutil.o flutim.o \
  gajeta.o aabro.o djan.o mnemo.o \
  fl_common.o fl_paths.o fl_ids.o fl_tools.o
DDEPS= \
  $(DEPS) \
  flu64.o fl_dispatcher.o
XDEPS= \
  $(DEPS) \
  dollar.o fl_executor.o fl_exe_nodes.o fl_exe_instructions.o
IDEPS= \
  $(DEPS) \
  dollar.o fl_invoker.o
LDEPS= \
  $(DEPS) $(SHV_OS) $(TSI_OS) \
  flu64.o fl_listener.o fl_lis_auth.o

_tmpsrc:
	cp -u ../src/*.[ch] src/
	cp -u ../src/n/*.[ch] src/
	cp -u ../src/lib/*.[ch] src/
	find ../spec/ -name "*_helpers.[ch]" -exec cp -u {} src/ \;
_tmpsrcc:
	rm -f src/*.[ch]
	$(MAKE) _tmpsrc

fl_exe_instructions.o: $(wildcard $(VPATH)/fl_n_*.c)
  # so that each time there is a change in fl_n_*.c, that .o gets rebuilt

cs.c: ../spec/c/*_spec.c
	$(RODZO) ../spec/c/*_spec.c -o cs.c
ds.c: ../spec/d/*_spec.c
	$(RODZO) ../spec/d/*_spec.c -o ds.c
xs.c: ../spec/x/*_spec.c
	$(RODZO) ../spec/x/*_spec.c -o xs.c
is.c: ../spec/i/*_spec.c
	$(RODZO) ../spec/i/*_spec.c -o is.c
ls.c: ../spec/l/*_spec.c
	$(RODZO) ../spec/l/*_spec.c -o ls.c
ns.c: ../spec/n/*_spec.c
	$(RODZO) ../spec/n/*_spec.c -o ns.c
zs.c: ../spec/z/*_spec.c
	$(RODZO) ../spec/z/*_spec.c -o zs.c

cs: $(DEPS)
ds: $(DDEPS) flon_helpers.o feu_helpers.o
xs: $(XDEPS)
is: $(IDEPS)
ls: $(LDEPS) fl_dispatcher.o l_helpers.o
ns: $(DEPS) feu_helpers.o
zs: $(DEPS) feu_helpers.o

s: cs ds xs is ls ns zs

warn:
	$(MAKE) _tmpsrc
	$(MAKE) clean cs ds xs is ls ns zs  2>&1 | . ./spec_filter.sh

sX:
	$(MAKE) _tmpsrc
	$(MAKE) $(deps)
	@echo "\n--- $(lastword $(deps)) ---------------------------------------\n"
	$(val) ./$(lastword $(deps))

sc: deps= cs
sc: sX
sd: deps= invoker executor ds
sd: sX
sx: deps= xs
sx: sX
si: deps= is
si: sX
sl: deps= invoker executor dispatcher ls
sl: sX
sn: deps= invoker executor dispatcher ns
sn: sX
sz: deps= invoker executor dispatcher zs
sz: sX

vsc: val= $(VAL)
vsc: sc
vsd: val= $(VAL)
vsd: sd
vsx: val= $(VAL)
vsx: sx
vsi: val= $(VAL)
vsi: si
vsl: val= $(VAL)
vsl: sl
vsn: val= $(VAL)
vsn: sn
vsz: val= $(VAL)
vsz: sz

sum:
	$(MAKE) -C .. ctst 2>&1 > /dev/null
	$(MAKE) _tmpsrc
	$(MAKE) invoker executor dispatcher cs ds is ls ns xs zs
	@echo "\n--- make sc ---common-------------------------------------------"
	@./cs 2>&1 | ./spec_filter.sh
	@echo "\n--- make sd ---dispatcher---------------------------------------"
	@./ds 2>&1 | ./spec_filter.sh
	@echo "\n--- make sx ---executor-----------------------------------------"
	@./xs 2>&1 | ./spec_filter.sh
	@echo "\n--- make si ---invoker------------------------------------------"
	@./is 2>&1 | ./spec_filter.sh
	@echo "\n--- make sl ---listener-----------------------------------------"
	@./ls 2>&1 | ./spec_filter.sh
	@echo "\n--- make sn ---instructions-------------------------------------"
	@./ns 2>&1 | ./spec_filter.sh
	@echo "\n--- make sz ---a-to-z-------------------------------------------"
	@./zs 2>&1 | ./spec_filter.sh
	@echo ""

vsum:
	$(MAKE) -C .. ctst 2>&1 > /dev/null
	$(MAKE) _tmpsrc
	$(MAKE) invoker executor dispatcher cs ds is ls ns xs zs
	@echo "\n--- make vsc ---common------------------------------------------"
	$(VAL) ./cs 2>&1 | ./spec_filter.sh
	@echo "\n--- make vsd ---dispatcher--------------------------------------"
	$(VAL) ./ds 2>&1 | ./spec_filter.sh
	@echo "\n--- make vsx ---executor----------------------------------------"
	$(VAL) ./xs 2>&1 | ./spec_filter.sh
	@echo "\n--- make vsi ---invoker-----------------------------------------"
	$(VAL) ./is 2>&1 | ./spec_filter.sh
	@echo "\n--- make vsl ---listener----------------------------------------"
	$(VAL) ./ls 2>&1 | ./spec_filter.sh
	@echo "\n--- make vsn ---instructions------------------------------------"
	$(VAL) ./ns 2>&1 | ./spec_filter.sh
	@echo "\n--- make vsz ---a-to-z------------------------------------------"
	$(VAL) ./zs 2>&1 | ./spec_filter.sh
	@echo ""

_specfilter:
	@echo ""
	@echo "** $(FIL):"
	@echo "*"
	@cat $(FIL) | ./spec_filter.sh x
vesn: ns invoker executor dispatcher
	FLONVAL=exe $(MAKE) sn
	find ../tst/var -name exe.log -exec $(MAKE) _specfilter FIL={} \;
vesz: ns invoker executor dispatcher
	FLONVAL=exe $(MAKE) sz
	find ../tst/var -name exe.log -exec $(MAKE) _specfilter FIL={} \;

ssi: _tmpsrc is
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

executor:
	$(MAKE) _tmpsrc; $(MAKE) flon-executor
	mv flon-executor ../tst/bin/
invoker:
	$(MAKE) _tmpsrc; $(MAKE) flon-invoker
	mv flon-invoker ../tst/bin/
dispatcher:
	$(MAKE) _tmpsrc; $(MAKE) flon-dispatcher
	mv flon-dispatcher ../tst/bin/
listener:
	$(MAKE) _tmpsrc; $(MAKE) flon-listener
	mv flon-listener ../tst/bin/
scope:
	$(MAKE) _tmpsrc; $(MAKE) flon-scope
	mv flon-scope ../tst/bin/
fflon:
	$(MAKE) _tmpsrc; $(MAKE) flon-flon
	mv flon-flon ../tst/bin/

spec:
	time -p $(MAKE) sum
vspec:
	time -p $(MAKE) vsum

.PHONY: clean _tmpsrc _tmpsrcc _specfilter

