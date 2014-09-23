
#default: $(NAME).o

.DEFAULT spec clean dispatcher executor listener:
	$(MAKE) -C tmp/ $@

# copy updated version of dep libs into src/
#
upgrade:
	cp -v ../shervin/src/*.{ch} src/

cs: clean spec

.PHONY: spec clean upgrade cs

