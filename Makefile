
#default: $(NAME).o

.DEFAULT spec clean dispatcher executor listener:
	$(MAKE) -C tmp/ $@

# copy up-to-date versions of dep libs into src/
#
upgrade:
	cp -v ../gajeta/src/*.[ch] src/
	cp -v ../djan/src/*.[ch] src/
	cp -v ../flutil/src/*.[ch] src/
	cp -v ../mnemo/src/*.[ch] src/
	rm src/todjan.c # no

ctst:
	find tst/var/ -name *.json | xargs rm -fv
	find tst/var/ -name *.txt | xargs rm -fv

.PHONY: spec clean upgrade ctst

