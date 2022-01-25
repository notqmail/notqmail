# Tests require
# 1. <https://pkg-config.freedesktop.org>
#    (or <https://github.com/pkgconf/pkgconf>)
# 2. <https://libcheck.github.io/check/>

SHELL=/bin/sh

default: it

.PHONY: clean default it unittest integrationtest test

UNITTESTS = unittest_stralloc unittest_blast unittest_prioq
INTEGRATIONTESTS = integrationtest_inject

clean:
	rm -f $(UNITTESTS) $(INTEGRATIONTESTS) *.o blast.c
	rm -rf testable

it: $(UNITTESTS) $(INTEGRATIONTESTS)

unittest: $(UNITTESTS)
	@for testprogram in $(UNITTESTS); do \
		./$$testprogram || exit 1; \
	done

integrationtest: $(INTEGRATIONTESTS)
	@for testprogram in $(INTEGRATIONTESTS); do \
		./$$testprogram || exit 2; \
	done

test: unittest integrationtest

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

unittest_prioq: \
../load unittest_prioq.o ../prioq.o ../error.a
	../load unittest_prioq ../prioq.o ../error.a \
	`pkg-config --libs check`

unittest_prioq.o: \
../compile unittest_prioq.c
	../compile unittest_prioq.c -I.. \
	`pkg-config --cflags check`

testable:
	( cd .. && mkdir -p tests/testable \
		&& cp Makefile TARGETS conf-* *.c *.h* *.sh tests/testable/ \
		&& mkdir tests/testable/gplv2 \
		&& cp gplv2/* tests/testable/gplv2/ )
	pwd > testable/conf-qmail

testable/qmail-inject: \
testable
	( cd testable && $(MAKE) qmail-inject )

testable/qmail-qfilter: \
testable
	( cd testable && $(MAKE) qmail-qfilter )

integrationtest_inject: \
testable/qmail-inject testable/qmail-qfilter integrationtest_inject.sh
	cp integrationtest_inject.sh integrationtest_inject
	chmod 755 integrationtest_inject
