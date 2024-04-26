# Tests require
# 1. <https://pkg-config.freedesktop.org>
#    (or <https://github.com/pkgconf/pkgconf>)
# 2. <https://libcheck.github.io/check/>

SHELL=/bin/sh

default: it

.PHONY: clean default it test

TESTBINS = unittest_byte unittest_case unittest_qmail-remote unittest_prioq unittest_qmail-send unittest_str unittest_stralloc

clean:
	rm -f $(TESTBINS) *.o *-without-main.c

it: $(TESTBINS)

test: it
	@for tbin in $(TESTBINS); do \
		./$$tbin || exit 1 ; \
	done

unittest_byte: \
../load unittest_byte.o ../str.a
	../load unittest_byte ../str.a \
	$(CHECK_LIBS)

unittest_byte.o: \
../compile unittest_byte.c ../byte.h
	../compile unittest_byte.c -I.. \
	$(CHECK_INCLUDES)

unittest_case: \
../load unittest_case.o ../case.a
	../load unittest_case ../case.a \
	`pkg-config --libs check`

unittest_case.o: \
../compile unittest_case.c ../case.h
	../compile unittest_case.c -I.. \
	`pkg-config --cflags check`

unittest_str: \
../load unittest_str.o ../str.a
	../load unittest_str ../str.a \
	`pkg-config --libs check`

unittest_str.o: \
../compile unittest_str.c ../str.h
	../compile unittest_str.c -I.. \
	`pkg-config --cflags check`

unittest_stralloc: \
../load unittest_stralloc.o ../stralloc.a ../str.a ../error.a
	../load unittest_stralloc ../stralloc.a ../str.a ../error.a \
	`pkg-config --libs check`

unittest_stralloc.o: \
../compile unittest_stralloc.c ../alloc.h ../stralloc.h
	../compile unittest_stralloc.c -I.. \
	`pkg-config --cflags check`

qmail-remote-without-main.c: \
../qmail-remote.c
	head -n $$(grep -n '^int main(' ../qmail-remote.c | sed 's/:.*//') ../qmail-remote.c | sed '/^int main(/d' > qmail-remote-without-main.c

qmail-remote-without-main.o: \
../compile qmail-remote-without-main.c
	../compile qmail-remote-without-main.c -I..

unittest_qmail-remote: \
../load unittest_qmail-remote.o qmail-remote-without-main.o ../control.o ../ip.o ../constmap.o \
../timeoutread.o ../timeoutwrite.o ../quote.o \
../stralloc.a ../str.a ../error.a ../substdio.a ../fs.a ../open.a ../str.a \
../getln.a ../case.a
	../load unittest_qmail-remote qmail-remote-without-main.o ../control.o ../ip.o ../constmap.o \
	../timeoutread.o ../timeoutwrite.o ../quote.o \
	../stralloc.a ../str.a ../error.a ../substdio.a ../fs.a ../open.a \
	../getln.a ../str.a ../case.a \
	`pkg-config --libs check`

unittest_qmail-remote.o: \
../compile unittest_qmail-remote.c ../alloc.h ../stralloc.h
	../compile unittest_qmail-remote.c -I.. \
	`pkg-config --cflags check`

unittest_prioq: \
../load unittest_prioq.o ../prioq.o ../error.a
	../load unittest_prioq ../prioq.o ../error.a \
	`pkg-config --libs check`

unittest_prioq.o: \
../compile unittest_prioq.c ../prioq.h
	../compile unittest_prioq.c -I.. \
	`pkg-config --cflags check`

qmail-send-without-main.c: \
../qmail-send.c
	head -n $$(grep -n '^int main(' ../qmail-send.c | sed 's/:.*//') ../qmail-send.c | sed '/^int main(/d' > qmail-send-without-main.c

unittest_qmail-send: \
../load unittest_qmail-send.o ../qsutil.o ../control.o ../constmap.o ../newfield.o ../prioq.o \
../trigger.o ../fmtqfn.o ../quote.o ../readsubdir.o ../qmail.o ../date822fmt.o \
../datetime.a ../case.a ../ndelay.a ../getln.a ../wait.a ../fd.a ../sig.a ../open.a \
../lock.a ../stralloc.a ../substdio.a ../error.a ../str.a ../fs.a ../auto_qmail.o \
../auto_split.o ../env.a
	../load unittest_qmail-send \
	../qsutil.o ../control.o ../constmap.o ../newfield.o \
	../prioq.o ../trigger.o ../fmtqfn.o ../quote.o ../readsubdir.o \
	../qmail.o ../date822fmt.o ../datetime.a ../case.a ../ndelay.a ../getln.a \
	../wait.a ../fd.a ../sig.a ../open.a ../lock.a ../stralloc.a \
	../substdio.a ../error.a ../str.a ../fs.a ../auto_qmail.o ../auto_split.o ../env.a \
	`pkg-config --libs check`

unittest_qmail-send.o: \
../compile unittest_qmail-send.c qmail-send-without-main.c
	../compile unittest_qmail-send.c -I.. \
	`pkg-config --cflags check`
