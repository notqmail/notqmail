# Tests require
# 1. <https://pkg-config.freedesktop.org>
#    (or <https://github.com/pkgconf/pkgconf>)
# 2. <https://libcheck.github.io/check/>

SHELL=/bin/sh

default: it

.PHONY: clean default it test

TESTBINS = unittest_stralloc

clean:
	rm -f $(TESTBINS) *.o

it: $(TESTBINS)

test: it
	@for tbin in $(TESTBINS); do \
		./$$tbin || exit 1 ; \
	done

unittest_stralloc: \
../load unittest_stralloc.o ../stralloc.a ../alloc.a ../str.a ../error.a
	../load unittest_stralloc ../stralloc.a ../alloc.a ../str.a ../error.a \
	`pkg-config --libs check`

unittest_stralloc.o: \
../compile unittest_stralloc.c ../alloc.h ../stralloc.h
	../compile unittest_stralloc.c -I.. \
	`pkg-config --cflags check`
