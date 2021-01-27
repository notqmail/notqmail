# Tests require
# 1. <https://pkg-config.freedesktop.org>
#    (or <https://github.com/pkgconf/pkgconf>)
# 2. <https://libcheck.github.io/check/>

SHELL=/bin/sh

default: it

.PHONY: clean default it test

TESTBINS = unittest_blast \
	unittest_case \
	unittest_ehlo_parse \
	unittest_prioq \
	unittest_smtpcode \
	unittest_str \
	unittest_stralloc

CHECK_INCLUDES := `pkg-config --cflags check`
CHECK_LIBS := `pkg-config --libs check`

clean:
	rm -f $(TESTBINS) *.o blast.c

it: $(TESTBINS)

test: it
	@for tbin in $(TESTBINS); do \
		./$$tbin || exit 1 ; \
	done

unittest_case: \
../load unittest_case.o ../case.a
	../load unittest_case ../case.a \
	$(CHECK_LIBS)

unittest_case.o: \
../compile unittest_case.c ../case.h
	../compile unittest_case.c -I.. \
	$(CHECK_INCLUDES)

unittest_ehlo_parse: \
../load unittest_ehlo_parse.o ../ehlo_parse.o ../ehlo_size.o \
../stralloc.a ../case.a ../str.a
	../load unittest_ehlo_parse ../ehlo_parse.o ../ehlo_size.o \
	../stralloc.a ../case.a ../str.a \
	$(CHECK_LIBS)

unittest_ehlo_parse.o: \
unittest_ehlo_parse.c ../compile ../ehlo_parse.h ../stralloc.h
	../compile $< -I.. \
	$(CHECK_INCLUDES)

unittest_str: \
../load unittest_str.o ../str.a
	../load unittest_str ../str.a \
	$(CHECK_LIBS)

unittest_str.o: \
../compile unittest_str.c ../str.h
	../compile unittest_str.c -I.. \
	$(CHECK_INCLUDES)

unittest_stralloc: \
../load unittest_stralloc.o ../stralloc.a ../str.a ../error.a
	../load unittest_stralloc ../stralloc.a ../str.a ../error.a \
	$(CHECK_LIBS)

unittest_stralloc.o: \
../compile unittest_stralloc.c ../alloc.h ../stralloc.h
	../compile unittest_stralloc.c -I.. \
	$(CHECK_INCLUDES)

blast.c: ../qmail-remote.c
	`head -n $$(grep -n '^int main(' ../qmail-remote.c | sed 's/:.*//') ../qmail-remote.c | sed '/^int main(/d' > blast.c`

blast.o: ../compile blast.c
	../compile blast.c -I..

unittest_blast: \
../load unittest_blast.o blast.o ../control.o ../ip.o ../constmap.o \
../ehlo_parse.o ../timeoutread.o ../timeoutwrite.o ../quote.o ../smtpcode.o \
../stralloc.a ../str.a ../error.a ../substdio.a ../fs.a ../open.a ../str.a \
../getln.a ../case.a
	../load unittest_blast blast.o ../control.o ../ip.o ../constmap.o \
	../ehlo_parse.o ../timeoutread.o ../timeoutwrite.o ../quote.o ../smtpcode.o \
	../stralloc.a ../str.a ../error.a ../substdio.a ../fs.a ../open.a \
	../getln.a ../str.a ../case.a \
	$(CHECK_LIBS)

unittest_blast.o: \
../compile unittest_blast.c ../alloc.h ../stralloc.h
	../compile unittest_blast.c -I.. \
	$(CHECK_INCLUDES)

unittest_prioq: \
../load unittest_prioq.o ../prioq.o ../error.a
	../load unittest_prioq ../prioq.o ../error.a \
	$(CHECK_LIBS)

unittest_prioq.o: \
../compile unittest_prioq.c
	../compile unittest_prioq.c -I.. \
	$(CHECK_INCLUDES)

unittest_smtpcode: \
../load unittest_smtpcode.o ../smtpcode.o ../stralloc.a ../substdio.a ../str.a
	../load unittest_smtpcode ../smtpcode.o ../stralloc.a ../substdio.a ../str.a \
	$(CHECK_LIBS)

unittest_smtpcode.o: \
../compile unittest_smtpcode.c ../smtpcode.h ../stralloc.h ../substdio.h
	../compile unittest_smtpcode.c -I.. \
	$(CHECK_INCLUDES)
