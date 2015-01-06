
NAME=flon

#default: $(NAME).o

.DEFAULT spec clean dispatcher executor listener:
	$(MAKE) -C tmp/ $@

# copy up-to-date versions of dep libs into src/
#
stamp:
	cd $(REP) && git log -n 1 | sed 's/^/\/\//' >> ../$(NAME)/$(FIL)
upgrade:
	cp -v ../shervin/src/*.[ch] src/lib/
	cp -v ../gajeta/src/*.[ch] src/lib/
	cp -v ../djan/src/*.[ch] src/lib/
	cp -v ../flutil/src/*.[ch] src/lib/
	cp -v ../mnemo/src/*.[ch] src/lib/
	cp -v ../tsifro/src/*.[ch] src/lib/
	cp -v ../dollar/src/dollar.[ch] src/lib/
	rm src/lib/todjan.c # no
	find src/lib/sh*.[ch] -exec $(MAKE) --quiet stamp REP=../shervin FIL={} \;
	find src/lib/gaj*.[ch] -exec $(MAKE) --quiet stamp REP=../gajeta FIL={} \;
	find src/lib/djan.[ch] -exec $(MAKE) --quiet stamp REP=../djan FIL={} \;
	find src/lib/flu*.[ch] -exec $(MAKE) --quiet stamp REP=../flutil FIL={} \;
	find src/lib/mnemo.[ch] -exec $(MAKE) --quiet stamp REP=../mnemo FIL={} \;
	find src/lib/tsifro.[ch] -exec $(MAKE) --quiet stamp REP=../tsifro FIL={} \;
	find src/lib/dollar.[ch] -exec $(MAKE) --quiet stamp REP=../dollar FIL={} \;

ctst:
	rm -f tst/var/spool/dis/*.json
	rm -f tst/var/spool/exe/*.json
	rm -f tst/var/spool/inv/*.json
	rm -f tst/var/spool/rejected/nada
	rm -f tst/var/spool/rejected/*.json
	rm -f tst/var/spool/rejected/*.jon
	find tst/var/spool/tdis/ -mindepth 1 -maxdepth 1 -type d | xargs rm -fR
	find tst/var/run/ -mindepth 1 -maxdepth 1 -type d | xargs rm -fR
	find tst/var/archive/ -mindepth 1 -maxdepth 1 -type d | xargs rm -fR
	find tst/var/log/ -mindepth 1 -maxdepth 1 -type d | xargs rm -fR
	rm -f tst/var/run/*.pid
	#rm -f tst/var/log/dispatcher.log
	echo "" > tst/var/log/dispatcher.log

dis:
	make clean dispatcher && \
    ./tst/bin/flon-dispatcher -d tst/
vdis:
	make clean dispatcher && \
    valgrind --leak-check=full -v ./tst/bin/flon-dispatcher -d tst/
lis:
	make clean listener && \
    ./tst/bin/flon-listener -d tst/
vlis:
	make clean listener && \
    valgrind --leak-check=full -v ./tst/bin/flon-listener -d tst/

.PHONY: spec clean upgrade ctst dis vdis lis vlis

