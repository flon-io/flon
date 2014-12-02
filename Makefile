
#default: $(NAME).o

.DEFAULT spec clean dispatcher executor listener:
	$(MAKE) -C tmp/ $@

# copy up-to-date versions of dep libs into src/
#
upgrade:
	cp -v ../shervin/src/*.[ch] src/
	cp -v ../gajeta/src/*.[ch] src/
	cp -v ../djan/src/*.[ch] src/
	cp -v ../flutil/src/*.[ch] src/
	cp -v ../mnemo/src/*.[ch] src/
	cp -v ../tsifro/src/*.[ch] src/
	cp -v ../dollar/src/dollar.[ch] src/
	rm src/todjan.c # no

ctst:
	rm -f tst/var/spool/dis/*.json
	rm -f tst/var/spool/exe/*.json
	rm -f tst/var/spool/inv/*.json
	rm -f tst/var/spool/rejected/nada
	rm -f tst/var/spool/rejected/*.json
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

