JUNK = $(shell git status -suall * | grep '^??' | sed 's/^?? //')

all:
	@echo Use make clean to remove junk

clean:
	rm -f $(JUNK)

.PHONY: clean
