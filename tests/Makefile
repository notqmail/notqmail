# Tests require
# 1. <https://pkg-config.freedesktop.org>
#    (or <https://github.com/pkgconf/pkgconf>)
# 2. <https://libcheck.github.io/check/>

SHELL=/bin/sh

default: it

.PHONY: clean default it test

TESTBINS = unittest_stralloc unittest_blast

clean:
	rm -f $(TESTBINS) *.o blast.c

it: $(TESTBINS)

test: it
	@for tbin in $(TESTBINS); do \
		./$$tbin || exit 1 ; \
	done

unittest_stralloc: \
../load unittest_stralloc.o ../stralloc.a ../str.a ../error.a
	../load unittest_stralloc ../stralloc.a ../str.a ../error.a \
	`pkg-config --libs check`

unittest_stralloc.o: \
../compile unittest_stralloc.c ../alloc.h ../stralloc.h
	../compile unittest_stralloc.c -I.. \
	`pkg-config --cflags check`

blast.c: ../qmail-remote.c
	`head -n $$(grep -n '^int main(' ../qmail-remote.c | sed 's/:.*//') ../qmail-remote.c | sed '/^int main(/d' > blast.c`

blast.o: ../compile blast.c
	../compile blast.c -I..

unittest_blast: \
../load unittest_blast.o blast.o ../control.o ../ip.o ../constmap.o \
../timeoutread.o ../timeoutwrite.o ../quote.o \
../stralloc.a ../str.a ../error.a ../substdio.a ../fs.a ../open.a ../str.a \
../getln.a ../case.a
	../load unittest_blast blast.o ../control.o ../ip.o ../constmap.o \
	../timeoutread.o ../timeoutwrite.o ../quote.o \
	../stralloc.a ../str.a ../error.a ../substdio.a ../fs.a ../open.a \
	../getln.a ../str.a ../case.a \
	`pkg-config --libs check`

unittest_blast.o: \
../compile unittest_blast.c ../alloc.h ../stralloc.h
	../compile unittest_blast.c -I.. \
	`pkg-config --cflags check`
