
NAME=sgmail
FLOND=../../../../..

default: m

.DEFAULT spec clean:
	$(MAKE) -C tmp/ $@ NAME=$(NAME)

upgrade:
	cp $(FLOND)/../bocla/src/bocla.* src
	cp $(FLOND)/src/flutil.* src
	cp $(FLOND)/src/aabro.* src
	cp $(FLOND)/src/djan.* src

#.PHONY: clean

