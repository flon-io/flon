
NAME=sgmail
FLOND=../../../../..

default: m

.DEFAULT spec clean:
	$(MAKE) -C tmp/ $@ NAME=$(NAME)

upgrade:
	cp $(FLOND)/../bocla/src/bocla.* src
	cp $(FLOND)/../flutil/src/flutil.* src

#.PHONY: clean

