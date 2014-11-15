
NAME=null
FLOND=../../../../..

default: n

.DEFAULT spec clean:
	$(MAKE) -C tmp/ $@ NAME=$(NAME)

upgrade:
	cp $(FLOND)/src/flutil.* src
	#cp $(FLOND)/src/aabro.* src
	#cp $(FLOND)/src/djan.* src

#.PHONY: clean

