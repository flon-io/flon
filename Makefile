
#default: $(NAME).o

.DEFAULT spec clean dispatcher executor listener:
	$(MAKE) -C tmp/ $@

# copy up-to-date versions of dep libs into src/
#
upgrade:
	cp -v ../gajeta/src/*.[ch] src/
	cp -v ../djan/src/*.[ch] src/
	cp -v ../flutil/src/*.[ch] src/

cs: clean spec

ctst:
	rm -f tst/var/log/invocations/*.txt
	rm -f tst/var/spool/in/*.json

.PHONY: spec clean upgrade cs ctst

