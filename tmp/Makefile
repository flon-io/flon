
VPATH=src
#CFLAGS=-I../src -g -Wall -O3 -lrt
#CFLAGS=-I../src -g -Wall -lrt -lev -D_POSIX_C_SOURCE=200809L
CFLAGS=-std=c99 -I$(VPATH) -g -Wall -lrt -lev
LDLIBS=
CC=gcc

VAL=valgrind --leak-check=full -v
#VAL=valgrind --leak-check=full -v --num-callers=50
#VAL=valgrind --read-var-info=yes --track-origins=yes --leak-check=full -v
#VAL=valgrind --vgdb-error=0 --leak-check=full -v

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
TDEPS= \
  $(DEPS) $(XDEPS) \
  fl_tasker.o
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
ts.c: ../spec/t/*_spec.c
	$(RODZO) ../spec/t/*_spec.c -o ts.c
ls.c: ../spec/l/*_spec.c
	$(RODZO) ../spec/l/*_spec.c -o ls.c
ns.c: ../spec/n/*_spec.c
	$(RODZO) ../spec/n/*_spec.c -o ns.c
zs.c: ../spec/z/*_spec.c
	$(RODZO) ../spec/z/*_spec.c -o zs.c

cs: $(DEPS)
ds: $(DDEPS) flon_helpers.o feu_helpers.o
xs: $(XDEPS)
ts: $(TDEPS) flon_helpers.o
ls: $(LDEPS) fl_dispatcher.o l_helpers.o
ns: $(DEPS) feu_helpers.o
zs: $(DEPS) feu_helpers.o

s: cs ds xs ts ls ns zs

warn:
	$(MAKE) _tmpsrc
	$(MAKE) clean cs ds xs ts ls ns zs  2>&1 | ruby specfilter.rb

sX:
	$(MAKE) _tmpsrc
	$(MAKE) $(_PREQ)
	@echo "\n--- $(lastword $(_PREQ)) --------------------------------------\n"
	$(_VAL) ./$(lastword $(_PREQ))
#
sc:
	$(MAKE) sX _VAL="$(_VAL)" _PREQ="cs"
sd:
	$(MAKE) sX _VAL="$(_VAL)" _PREQ="tasker executor ds"
sx:
	$(MAKE) sX _VAL="$(_VAL)" _PREQ="xs"
st:
	$(MAKE) sX _VAL="$(_VAL)" _PREQ="ts"
sl:
	$(MAKE) sX _VAL="$(_VAL)" _PREQ="tasker executor dispatcher ls"
sn:
	$(MAKE) sX _VAL="$(_VAL)" _PREQ="tasker executor dispatcher ns"
sz:
	$(MAKE) sX _VAL="$(_VAL)" _PREQ="tasker executor dispatcher zs"
#
vsc:
	$(MAKE) sc _VAL="$(VAL)"
vsd:
	$(MAKE) sd _VAL="$(VAL)"
vsx:
	$(MAKE) sx _VAL="$(VAL)"
vst:
	$(MAKE) st _VAL="$(VAL)"
vsl:
	$(MAKE) sl _VAL="$(VAL)"
vsn:
	$(MAKE) sn _VAL="$(VAL)"
vsz:
	$(MAKE) sz _VAL="$(VAL)"


sum:
	$(MAKE) -C .. ctst 2>&1 > /dev/null
	$(MAKE) _tmpsrc
	$(MAKE) tasker executor dispatcher cs ds ts ls ns xs zs
	@echo "\n--- make sc ---common-------------------------------------------"
	@./cs 2>&1 | ruby specfilter.rb
	@echo "\n--- make sd ---dispatcher---------------------------------------"
	@./ds 2>&1 | ruby specfilter.rb
	@echo "\n--- make sx ---executor-----------------------------------------"
	@./xs 2>&1 | ruby specfilter.rb
	@echo "\n--- make st ---tasker-------------------------------------------"
	@./ts 2>&1 | ruby specfilter.rb
	@echo "\n--- make sl ---listener-----------------------------------------"
	@./ls 2>&1 | ruby specfilter.rb
	@echo "\n--- make sn ---instructions-------------------------------------"
	@./ns 2>&1 | ruby specfilter.rb
	@echo "\n--- make sz ---a-to-z-------------------------------------------"
	@./zs 2>&1 | ruby specfilter.rb
	@echo "\n---"
	@echo ""

vsum:
	$(MAKE) -C .. ctst 2>&1 > /dev/null
	$(MAKE) _tmpsrc
	$(MAKE) tasker executor dispatcher cs ds ts ls ns xs zs
	@echo "\n--- make vsc ---common------------------------------------------"
	$(VAL) ./cs 2>&1 | ruby specfilter.rb
	@echo "\n--- make vsd ---dispatcher--------------------------------------"
	$(VAL) ./ds 2>&1 | ruby specfilter.rb
	@echo "\n--- make vsx ---executor----------------------------------------"
	$(VAL) ./xs 2>&1 | ruby specfilter.rb
	@echo "\n--- make vst ---tasker------------------------------------------"
	$(VAL) ./ts 2>&1 | ruby specfilter.rb
	@echo "\n--- make vsl ---listener----------------------------------------"
	$(VAL) ./ls 2>&1 | ruby specfilter.rb
	@echo "\n--- make vsn ---instructions------------------------------------"
	$(VAL) ./ns 2>&1 | ruby specfilter.rb
	@echo "\n--- make vsz ---a-to-z------------------------------------------"
	$(VAL) ./zs 2>&1 | ruby specfilter.rb
	@echo ""

vesn: ns tasker executor dispatcher
	FLONVAL=exe $(MAKE) sn
	#find ../tst/var -name exe.log -exec $(MAKE) _specfilter FIL={} \;
	find ../tst/var -name exe.log -exec sh -c "cat {} | ruby specfilter.rb {}" \;
vesz: ns tasker executor dispatcher
	FLONVAL=exe $(MAKE) sz
	#find ../tst/var -name exe.log -exec $(MAKE) _specfilter FIL={} \;
	find ../tst/var -name exe.log -exec sh -c "cat {} | ruby specfilter.rb {}" \;

sst: _tmpsrc ts
	strace -r ./ts
ssd: ds executor tasker
	strace -r ./ds

flon-executor: $(XDEPS)
flon-tasker: $(TDEPS)
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
tasker:
	$(MAKE) _tmpsrc; $(MAKE) flon-tasker
	mv flon-tasker ../tst/bin/
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

.PHONY: clean _tmpsrc _tmpsrcc

