
#default: $(NAME).o

.DEFAULT spec clean dispatcher executor listener:
	$(MAKE) -C tmp/ $@

# copy up-to-date versions of dep libs into src/
#
upgrade:
	cp -v ../djan/src/*.[ch] src/
	#cp -v ../shervin/src/*.[ch] src/

cs: clean spec

.PHONY: spec clean upgrade cs

