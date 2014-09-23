
NAME=xxx

default: $(NAME).o

.DEFAULT spec clean:
	$(MAKE) -C tmp/ $@ NAME=$(NAME)

# copy updated version of dep libs into src/
#
upgrade:
	cp -v ../shervin/src/*.{ch} src/

cs: clean spec

.PHONY: spec clean upgrade cs

