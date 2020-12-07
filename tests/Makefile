# Tests require
# 1. <https://pkg-config.freedesktop.org>
#    (or <https://github.com/pkgconf/pkgconf>)
# 2. <https://libcheck.github.io/check/>

SHELL=/bin/sh

default: it

.PHONY: clean default it test

TESTBINS = unittest_stralloc unittest_blast unittest_prioq \
		   unittest_utf8read

clean:
	rm -f $(TESTBINS) *.o blast.c get_capability.c smtpcode.c \
	mailfrom_parms.c

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
../timeoutread.o ../timeoutwrite.o ../quote.o ../utf8read.o \
../stralloc.a ../str.a ../error.a ../substdio.a ../fs.a ../open.a ../str.a \
../getln.a ../case.a
	../load unittest_blast blast.o ../control.o ../ip.o ../constmap.o \
	../timeoutread.o ../timeoutwrite.o ../quote.o ../utf8read.o \
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

unittest_utf8read: \
../load unittest_utf8read.o ../utf8read.o ../stralloc.a ../open.a \
get_capability.o smtpcode.o ../scan_ulong.o ../str.a \
mailfrom_parms.o ../substdio.a ../case.a
	../load unittest_utf8read ../utf8read.o get_capability.o \
	mailfrom_parms.o smtpcode.o ../scan_ulong.o ../stralloc.a \
	../open.a ../substdio.a ../case.a ../str.a \
	`pkg-config --libs check`

unittest_utf8read.o: \
../compile unittest_utf8read.c ../open.h ../str.h \
../seek.h ../utf8read.h ../substdio.h ../subfd.h
	../compile -I.. unittest_utf8read.c -I.. \
	`pkg-config --cflags check`

get_capability.o: \
../compile get_capability.c
	../compile -I.. get_capability.c

get_capability.c: \
../qmail-remote.c ./get_function
	(echo "#include \"str.h\""; \
	echo "#include \"case.h\""; \
	echo "#include \"stralloc.h\""; \
	echo "extern stralloc smtptext;"; \
	./get_function "int get_capability" \
		../qmail-remote.c) \
		> get_capability.c

smtpcode.o: \
../compile smtpcode.c
	../compile -I.. smtpcode.c

smtpcode.c: \
../qmail-remote.c ./get_function
	(\
	echo "#include \"stralloc.h\""; \
	echo "#include \"substdio.h\""; \
	echo "#include \"scan.h\""; \
	echo "#define HUGESMTPTEXT 5000"; \
	echo "extern stralloc smtptext;"; \
	echo "extern substdio smtpfrom;"; \
	echo "extern void temp_nomem();"; \
	./get_function "static void get1" ../qmail-remote.c; \
	./get_function "static unsigned long get3" ../qmail-remote.c; \
	./get_function "unsigned long smtpcode" ../qmail-remote.c) > smtpcode.c

mailfrom_parms.o: \
../compile mailfrom_parms.c
	../compile -I.. mailfrom_parms.c

mailfrom_parms.c: \
../qmail-smtpd.c ./get_function
	(\
	echo "#include \"stralloc.h\""; \
	echo "#include \"str.h\""; \
	echo "#include \"byte.h\""; \
	echo "#include \"case.h\""; \
	echo "extern stralloc mfparms;"; \
	echo "extern int smtputf8;"; \
	echo "extern void temp_nomem();"; \
	./get_function "void mailfrom_parms" \
	../qmail-smtpd.c|sed s}die_nomem}temp_nomem}g) > mailfrom_parms.c
