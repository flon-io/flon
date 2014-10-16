
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

ctst:
	rm -f tst/var/log/exe/*.txt
	rm -f tst/var/log/inv/*.txt
	rm -f tst/var/spool/dis/*.json
	rm -f tst/var/spool/exe/*.json
	rm -f tst/var/spool/inv/*.json
	rm -f tst/var/spool/rejected/*.json
	rm -f tst/var/spool/processed/*.json

.PHONY: spec clean upgrade ctst

