
NAME=null
FLOND=../../../../..

default: n

.DEFAULT spec clean:
	$(MAKE) -C tmp/ $@ NAME=$(NAME)

upgrade:
	cp $(FLOND)/src/fluti*.[ch] src
	cp $(FLOND)/src/gajeta.[ch] src

#.PHONY: clean

