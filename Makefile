# Don't edit Makefile! Use conf-* for configuration.

SHELL=/bin/sh

default: it

addresses.0: \
addresses.5
	nroff -man addresses.5 > addresses.0

alloc.a: \
makelib alloc.o alloc_re.o
	./makelib alloc.a alloc.o alloc_re.o

alloc.o: \
compile alloc.c alloc.h error.h
	./compile alloc.c

alloc_re.o: \
compile alloc_re.c alloc.h byte.h
	./compile alloc_re.c

auto-ccld.sh: \
conf-cc conf-ld warn-auto.sh
	( cat warn-auto.sh; \
	echo CC=\'`head -1 conf-cc`\'; \
	echo LD=\'`head -1 conf-ld`\' \
	) > auto-ccld.sh

auto-gid: \
load auto-gid.o substdio.a error.a str.a fs.a
	./load auto-gid substdio.a error.a str.a fs.a 

auto-gid.o: \
compile auto-gid.c subfd.h substdio.h substdio.h readwrite.h exit.h \
scan.h fmt.h
	./compile auto-gid.c

auto-int: \
load auto-int.o substdio.a error.a str.a fs.a
	./load auto-int substdio.a error.a str.a fs.a 

auto-int.o: \
compile auto-int.c substdio.h readwrite.h exit.h scan.h fmt.h
	./compile auto-int.c

auto-int8: \
load auto-int8.o substdio.a error.a str.a fs.a
	./load auto-int8 substdio.a error.a str.a fs.a 

auto-int8.o: \
compile auto-int8.c substdio.h readwrite.h exit.h scan.h fmt.h
	./compile auto-int8.c

auto-str: \
load auto-str.o substdio.a error.a str.a
	./load auto-str substdio.a error.a str.a 

auto-str.o: \
compile auto-str.c substdio.h readwrite.h exit.h
	./compile auto-str.c

auto-uid: \
load auto-uid.o substdio.a error.a str.a fs.a
	./load auto-uid substdio.a error.a str.a fs.a 

auto-uid.o: \
compile auto-uid.c subfd.h substdio.h substdio.h readwrite.h exit.h \
scan.h fmt.h
	./compile auto-uid.c

auto_break.c: \
auto-str conf-break
	./auto-str auto_break \
	"`head -1 conf-break`" > auto_break.c

auto_break.o: \
compile auto_break.c
	./compile auto_break.c

auto_patrn.c: \
auto-int8 conf-patrn
	./auto-int8 auto_patrn `head -1 conf-patrn` > auto_patrn.c

auto_patrn.o: \
compile auto_patrn.c
	./compile auto_patrn.c

auto_qmail.c: \
auto-str conf-qmail
	./auto-str auto_qmail `head -1 conf-qmail` > auto_qmail.c

auto_qmail.o: \
compile auto_qmail.c
	./compile auto_qmail.c

auto_spawn.c: \
auto-int conf-spawn
	./auto-int auto_spawn `head -1 conf-spawn` > auto_spawn.c

auto_spawn.o: \
compile auto_spawn.c
	./compile auto_spawn.c

auto_split.c: \
auto-int conf-split
	./auto-int auto_split `head -1 conf-split` > auto_split.c

auto_split.o: \
compile auto_split.c
	./compile auto_split.c

auto_uids.c: \
auto-uid auto-gid conf-users conf-groups
	( ./auto-uid auto_uida `head -1 conf-users` \
	&&./auto-uid auto_uidd `head -2 conf-users | tail -1` \
	&&./auto-uid auto_uidl `head -3 conf-users | tail -1` \
	&&./auto-uid auto_uido `head -4 conf-users | tail -1` \
	&&./auto-uid auto_uidp `head -5 conf-users | tail -1` \
	&&./auto-uid auto_uidq `head -6 conf-users | tail -1` \
	&&./auto-uid auto_uidr `head -7 conf-users | tail -1` \
	&&./auto-uid auto_uids `head -8 conf-users | tail -1` \
	&&./auto-gid auto_gidq `head -1 conf-groups` \
	&&./auto-gid auto_gidn `head -2 conf-groups | tail -1` \
	) > auto_uids.c.tmp && mv auto_uids.c.tmp auto_uids.c

auto_uids.o: \
compile auto_uids.c
	./compile auto_uids.c

auto_usera.c: \
auto-str conf-users
	./auto-str auto_usera `head -1 conf-users` > auto_usera.c

auto_usera.o: \
compile auto_usera.c
	./compile auto_usera.c

binm1: \
binm1.sh conf-qmail
	cat binm1.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm1
	chmod 755 binm1

binm1+df: \
binm1+df.sh conf-qmail
	cat binm1+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm1+df
	chmod 755 binm1+df

binm2: \
binm2.sh conf-qmail
	cat binm2.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm2
	chmod 755 binm2

binm2+df: \
binm2+df.sh conf-qmail
	cat binm2+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm2+df
	chmod 755 binm2+df

binm3: \
binm3.sh conf-qmail
	cat binm3.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm3
	chmod 755 binm3

binm3+df: \
binm3+df.sh conf-qmail
	cat binm3+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm3+df
	chmod 755 binm3+df

bouncesaying: \
load bouncesaying.o strerr.a error.a substdio.a str.a wait.a
	./load bouncesaying strerr.a error.a substdio.a str.a \
	wait.a 

bouncesaying.0: \
bouncesaying.1
	nroff -man bouncesaying.1 > bouncesaying.0

bouncesaying.o: \
compile bouncesaying.c fork.h strerr.h error.h wait.h sig.h exit.h
	./compile bouncesaying.c

byte_chr.o: \
compile byte_chr.c byte.h
	./compile byte_chr.c

byte_copy.o: \
compile byte_copy.c byte.h
	./compile byte_copy.c

byte_cr.o: \
compile byte_cr.c byte.h
	./compile byte_cr.c

byte_diff.o: \
compile byte_diff.c byte.h
	./compile byte_diff.c

byte_rchr.o: \
compile byte_rchr.c byte.h
	./compile byte_rchr.c

byte_zero.o: \
compile byte_zero.c byte.h
	./compile byte_zero.c

case.a: \
makelib case_diffb.o case_diffs.o case_lowerb.o case_lowers.o \
case_starts.o
	./makelib case.a case_diffb.o case_diffs.o case_lowerb.o \
	case_lowers.o case_starts.o

case_diffb.o: \
compile case_diffb.c case.h
	./compile case_diffb.c

case_diffs.o: \
compile case_diffs.c case.h
	./compile case_diffs.c

case_lowerb.o: \
compile case_lowerb.c case.h
	./compile case_lowerb.c

case_lowers.o: \
compile case_lowers.c case.h
	./compile case_lowers.c

case_starts.o: \
compile case_starts.c case.h
	./compile case_starts.c

cdb.a: \
makelib cdb_hash.o cdb_unpack.o cdb_seek.o
	./makelib cdb.a cdb_hash.o cdb_unpack.o cdb_seek.o

cdb_hash.o: \
compile cdb_hash.c cdb.h uint32.h
	./compile cdb_hash.c

cdb_seek.o: \
compile cdb_seek.c cdb.h uint32.h
	./compile cdb_seek.c

cdb_unpack.o: \
compile cdb_unpack.c cdb.h uint32.h
	./compile cdb_unpack.c

cdbmake.a: \
makelib cdbmake_pack.o cdbmake_hash.o cdbmake_add.o
	./makelib cdbmake.a cdbmake_pack.o cdbmake_hash.o \
	cdbmake_add.o

cdbmake_add.o: \
compile cdbmake_add.c cdbmake.h uint32.h
	./compile cdbmake_add.c

cdbmake_hash.o: \
compile cdbmake_hash.c cdbmake.h uint32.h
	./compile cdbmake_hash.c

cdbmake_pack.o: \
compile cdbmake_pack.c cdbmake.h uint32.h
	./compile cdbmake_pack.c

cdbmss.o: \
compile cdbmss.c readwrite.h seek.h alloc.h cdbmss.h cdbmake.h \
uint32.h substdio.h
	./compile cdbmss.c

check: \
it man
	./instcheck

chkshsgr: \
load chkshsgr.o
	./load chkshsgr 

chkshsgr.o: \
compile chkshsgr.c exit.h
	./compile chkshsgr.c

chkspawn: \
load chkspawn.o substdio.a error.a str.a fs.a auto_spawn.o
	./load chkspawn substdio.a error.a str.a fs.a auto_spawn.o 

chkspawn.o: \
compile chkspawn.c substdio.h subfd.h substdio.h fmt.h select.h \
exit.h auto_spawn.h
	./compile chkspawn.c

clean: \
TARGETS
	rm -f `cat TARGETS`

coe.o: \
compile coe.c coe.h
	./compile coe.c

commands.o: \
compile commands.c commands.h substdio.h stralloc.h gen_alloc.h str.h \
case.h
	./compile commands.c

compile: \
make-compile warn-auto.sh systype
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

condredirect: \
load condredirect.o qmail.o strerr.a fd.a sig.a wait.a seek.a env.a \
substdio.a error.a str.a fs.a auto_qmail.o
	./load condredirect qmail.o strerr.a fd.a sig.a wait.a \
	seek.a env.a substdio.a error.a str.a fs.a auto_qmail.o 

condredirect.0: \
condredirect.1
	nroff -man condredirect.1 > condredirect.0

condredirect.o: \
compile condredirect.c sig.h readwrite.h exit.h env.h error.h fork.h \
wait.h seek.h qmail.h substdio.h strerr.h substdio.h fmt.h
	./compile condredirect.c

config: \
warn-auto.sh config.sh conf-qmail conf-break conf-split
	cat warn-auto.sh config.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> config
	chmod 755 config

config-fast: \
warn-auto.sh config-fast.sh conf-qmail conf-break conf-split
	cat warn-auto.sh config-fast.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> config-fast
	chmod 755 config-fast

constmap.o: \
compile constmap.c constmap.h alloc.h case.h
	./compile constmap.c

control.o: \
compile control.c readwrite.h open.h getln.h stralloc.h gen_alloc.h \
substdio.h error.h control.h alloc.h scan.h
	./compile control.c

date822fmt.o: \
compile date822fmt.c datetime.h fmt.h date822fmt.h
	./compile date822fmt.c

datemail: \
warn-auto.sh datemail.sh conf-qmail conf-break conf-split
	cat warn-auto.sh datemail.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> datemail
	chmod 755 datemail

datetime.a: \
makelib datetime.o datetime_un.o
	./makelib datetime.a datetime.o datetime_un.o

datetime.o: \
compile datetime.c datetime.h
	./compile datetime.c

datetime_un.o: \
compile datetime_un.c datetime.h
	./compile datetime_un.c

direntry.h: \
compile trydrent.c direntry.h1 direntry.h2
	( ./compile trydrent.c >/dev/null 2>&1 \
	&& cat direntry.h2 || cat direntry.h1 ) > direntry.h
	rm -f trydrent.o

dns.lib: \
tryrsolv.c compile load socket.lib dns.o ipalloc.o ip.o stralloc.a \
alloc.a error.a fs.a str.a
	( ( ./compile tryrsolv.c && ./load tryrsolv dns.o \
	ipalloc.o ip.o stralloc.a alloc.a error.a fs.a str.a \
	-lresolv `cat socket.lib` ) >/dev/null 2>&1 \
	&& echo -lresolv || exit 0 ) > dns.lib
	rm -f tryrsolv.o tryrsolv

dns.o: \
compile dns.c ip.h ipalloc.h ip.h gen_alloc.h fmt.h alloc.h str.h \
stralloc.h gen_alloc.h dns.h case.h
	./compile dns.c

dnscname: \
load dnscname.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnscname dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnscname.o: \
compile dnscname.c substdio.h subfd.h substdio.h stralloc.h \
gen_alloc.h dns.h dnsdoe.h readwrite.h exit.h
	./compile dnscname.c

dnsdoe.o: \
compile dnsdoe.c substdio.h subfd.h substdio.h exit.h dns.h dnsdoe.h
	./compile dnsdoe.c

dnsfq: \
load dnsfq.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsfq dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnsfq.o: \
compile dnsfq.c substdio.h subfd.h substdio.h stralloc.h gen_alloc.h \
dns.h dnsdoe.h ip.h ipalloc.h ip.h gen_alloc.h exit.h
	./compile dnsfq.c

dnsip: \
load dnsip.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsip dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnsip.o: \
compile dnsip.c substdio.h subfd.h substdio.h stralloc.h gen_alloc.h \
dns.h dnsdoe.h ip.h ipalloc.h ip.h gen_alloc.h exit.h
	./compile dnsip.c

dnsmxip: \
load dnsmxip.o dns.o dnsdoe.o ip.o ipalloc.o now.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsmxip dns.o dnsdoe.o ip.o ipalloc.o now.o \
	stralloc.a alloc.a substdio.a error.a str.a fs.a  `cat \
	dns.lib` `cat socket.lib`

dnsmxip.o: \
compile dnsmxip.c substdio.h subfd.h substdio.h stralloc.h \
gen_alloc.h fmt.h dns.h dnsdoe.h ip.h ipalloc.h ip.h gen_alloc.h \
now.h datetime.h exit.h
	./compile dnsmxip.c

dnsptr: \
load dnsptr.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsptr dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnsptr.o: \
compile dnsptr.c substdio.h subfd.h substdio.h stralloc.h gen_alloc.h \
str.h scan.h dns.h dnsdoe.h ip.h exit.h
	./compile dnsptr.c

dot-qmail.0: \
dot-qmail.5
	nroff -man dot-qmail.5 > dot-qmail.0

dot-qmail.5: \
dot-qmail.9 conf-break conf-spawn
	cat dot-qmail.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> dot-qmail.5

elq: \
warn-auto.sh elq.sh conf-qmail conf-break conf-split
	cat warn-auto.sh elq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> elq
	chmod 755 elq

env.a: \
makelib env.o envread.o
	./makelib env.a env.o envread.o

env.o: \
compile env.c str.h alloc.h env.h
	./compile env.c

envelopes.0: \
envelopes.5
	nroff -man envelopes.5 > envelopes.0

envread.o: \
compile envread.c env.h str.h
	./compile envread.c

error.a: \
makelib error.o error_str.o error_temp.o
	./makelib error.a error.o error_str.o error_temp.o

error.o: \
compile error.c error.h
	./compile error.c

error_str.o: \
compile error_str.c error.h
	./compile error_str.c

error_temp.o: \
compile error_temp.c error.h
	./compile error_temp.c

except: \
load except.o strerr.a error.a substdio.a str.a wait.a
	./load except strerr.a error.a substdio.a str.a wait.a 

except.0: \
except.1
	nroff -man except.1 > except.0

except.o: \
compile except.c fork.h strerr.h wait.h error.h exit.h
	./compile except.c

fd.a: \
makelib fd_copy.o fd_move.o
	./makelib fd.a fd_copy.o fd_move.o

fd_copy.o: \
compile fd_copy.c fd.h
	./compile fd_copy.c

fd_move.o: \
compile fd_move.c fd.h
	./compile fd_move.c

fifo.o: \
compile fifo.c hasmkffo.h fifo.h
	./compile fifo.c

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

fmt_str.o: \
compile fmt_str.c fmt.h
	./compile fmt_str.c

fmt_strn.o: \
compile fmt_strn.c fmt.h
	./compile fmt_strn.c

fmt_uint.o: \
compile fmt_uint.c fmt.h
	./compile fmt_uint.c

fmt_uint0.o: \
compile fmt_uint0.c fmt.h
	./compile fmt_uint0.c

fmt_ulong.o: \
compile fmt_ulong.c fmt.h
	./compile fmt_ulong.c

fmtqfn.o: \
compile fmtqfn.c fmtqfn.h fmt.h auto_split.h
	./compile fmtqfn.c

forgeries.0: \
forgeries.7
	nroff -man forgeries.7 > forgeries.0

fork.h: \
compile load tryvfork.c fork.h1 fork.h2
	( ( ./compile tryvfork.c && ./load tryvfork ) >/dev/null \
	2>&1 \
	&& cat fork.h2 || cat fork.h1 ) > fork.h
	rm -f tryvfork.o tryvfork

forward: \
load forward.o qmail.o strerr.a alloc.a fd.a wait.a sig.a env.a \
substdio.a error.a str.a fs.a auto_qmail.o
	./load forward qmail.o strerr.a alloc.a fd.a wait.a sig.a \
	env.a substdio.a error.a str.a fs.a auto_qmail.o 

forward.0: \
forward.1
	nroff -man forward.1 > forward.0

forward.o: \
compile forward.c sig.h readwrite.h exit.h env.h qmail.h substdio.h \
strerr.h substdio.h fmt.h
	./compile forward.c

fs.a: \
makelib fmt_str.o fmt_strn.o fmt_uint.o fmt_uint0.o fmt_ulong.o \
scan_ulong.o scan_8long.o
	./makelib fs.a fmt_str.o fmt_strn.o fmt_uint.o fmt_uint0.o \
	fmt_ulong.o scan_ulong.o scan_8long.o

getln.a: \
makelib getln.o getln2.o
	./makelib getln.a getln.o getln2.o

getln.o: \
compile getln.c substdio.h byte.h stralloc.h gen_alloc.h getln.h
	./compile getln.c

getln2.o: \
compile getln2.c substdio.h stralloc.h gen_alloc.h byte.h getln.h
	./compile getln2.c

getopt.a: \
makelib subgetopt.o sgetopt.o
	./makelib getopt.a subgetopt.o sgetopt.o

gfrom.o: \
compile gfrom.c str.h gfrom.h
	./compile gfrom.c

hasflock.h: \
tryflock.c compile load
	( ( ./compile tryflock.c && ./load tryflock ) >/dev/null \
	2>&1 \
	&& echo \#define HASFLOCK 1 || exit 0 ) > hasflock.h
	rm -f tryflock.o tryflock

hasmkffo.h: \
trymkffo.c compile load
	( ( ./compile trymkffo.c && ./load trymkffo ) >/dev/null \
	2>&1 \
	&& echo \#define HASMKFIFO 1 || exit 0 ) > hasmkffo.h
	rm -f trymkffo.o trymkffo

hasnpbg1.h: \
trynpbg1.c compile load open.h open.a fifo.h fifo.o select.h
	( ( ./compile trynpbg1.c \
	&& ./load trynpbg1 fifo.o open.a && ./trynpbg1 ) \
	>/dev/null 2>&1 \
	&& echo \#define HASNAMEDPIPEBUG1 1 || exit 0 ) > \
	hasnpbg1.h
	rm -f trynpbg1.o trynpbg1

hassalen.h: \
trysalen.c compile
	( ./compile trysalen.c >/dev/null 2>&1 \
	&& echo \#define HASSALEN 1 || exit 0 ) > hassalen.h
	rm -f trysalen.o

hassgact.h: \
trysgact.c compile load
	( ( ./compile trysgact.c && ./load trysgact ) >/dev/null \
	2>&1 \
	&& echo \#define HASSIGACTION 1 || exit 0 ) > hassgact.h
	rm -f trysgact.o trysgact

hassgprm.h: \
trysgprm.c compile load
	( ( ./compile trysgprm.c && ./load trysgprm ) >/dev/null \
	2>&1 \
	&& echo \#define HASSIGPROCMASK 1 || exit 0 ) > hassgprm.h
	rm -f trysgprm.o trysgprm

hasshsgr.h: \
chkshsgr warn-shsgr tryshsgr.c compile load
	./chkshsgr || ( cat warn-shsgr; exit 1 )
	( ( ./compile tryshsgr.c \
	&& ./load tryshsgr && ./tryshsgr ) >/dev/null 2>&1 \
	&& echo \#define HASSHORTSETGROUPS 1 || exit 0 ) > \
	hasshsgr.h
	rm -f tryshsgr.o tryshsgr

haswaitp.h: \
trywaitp.c compile load
	( ( ./compile trywaitp.c && ./load trywaitp ) >/dev/null \
	2>&1 \
	&& echo \#define HASWAITPID 1 || exit 0 ) > haswaitp.h
	rm -f trywaitp.o trywaitp

headerbody.o: \
compile headerbody.c stralloc.h gen_alloc.h substdio.h getln.h \
hfield.h headerbody.h
	./compile headerbody.c

hfield.o: \
compile hfield.c hfield.h
	./compile hfield.c

hier.o: \
compile hier.c auto_qmail.h auto_split.h auto_uids.h fmt.h fifo.h
	./compile hier.c

home: \
home.sh conf-qmail
	cat home.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> home
	chmod 755 home

home+df: \
home+df.sh conf-qmail
	cat home+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> home+df
	chmod 755 home+df

hostname: \
load hostname.o substdio.a error.a str.a dns.lib socket.lib
	./load hostname substdio.a error.a str.a  `cat dns.lib` \
	`cat socket.lib`

hostname.o: \
compile hostname.c substdio.h subfd.h substdio.h readwrite.h exit.h
	./compile hostname.c

idedit: \
load idedit.o strerr.a substdio.a error.a str.a fs.a wait.a open.a \
seek.a
	./load idedit strerr.a substdio.a error.a str.a fs.a \
	wait.a open.a seek.a 

idedit.o: \
compile idedit.c readwrite.h exit.h scan.h fmt.h strerr.h open.h \
seek.h fork.h
	./compile idedit.c

install: \
load install.o fifo.o hier.o auto_qmail.o auto_split.o auto_uids.o \
strerr.a substdio.a open.a error.a str.a fs.a
	./load install fifo.o hier.o auto_qmail.o auto_split.o \
	auto_uids.o strerr.a substdio.a open.a error.a str.a fs.a 

install-big: \
load install-big.o fifo.o install.o auto_qmail.o auto_split.o \
auto_uids.o strerr.a substdio.a open.a error.a str.a fs.a
	./load install-big fifo.o install.o auto_qmail.o \
	auto_split.o auto_uids.o strerr.a substdio.a open.a error.a \
	str.a fs.a 

install-big.o: \
compile install-big.c auto_qmail.h auto_split.h auto_uids.h fmt.h \
fifo.h
	./compile install-big.c

install.o: \
compile install.c substdio.h strerr.h error.h open.h readwrite.h \
exit.h
	./compile install.c

instcheck: \
load instcheck.o fifo.o hier.o auto_qmail.o auto_split.o auto_uids.o \
strerr.a substdio.a error.a str.a fs.a
	./load instcheck fifo.o hier.o auto_qmail.o auto_split.o \
	auto_uids.o strerr.a substdio.a error.a str.a fs.a 

instcheck.o: \
compile instcheck.c strerr.h error.h readwrite.h exit.h
	./compile instcheck.c

ip.o: \
compile ip.c fmt.h scan.h ip.h
	./compile ip.c

ipalloc.o: \
compile ipalloc.c alloc.h gen_allocdefs.h ip.h ipalloc.h ip.h \
gen_alloc.h
	./compile ipalloc.c

ipme.o: \
compile ipme.c hassalen.h byte.h ip.h ipalloc.h ip.h gen_alloc.h \
stralloc.h gen_alloc.h ipme.h ip.h ipalloc.h
	./compile ipme.c

ipmeprint: \
load ipmeprint.o ipme.o ip.o ipalloc.o stralloc.a alloc.a substdio.a \
error.a str.a fs.a socket.lib
	./load ipmeprint ipme.o ip.o ipalloc.o stralloc.a alloc.a \
	substdio.a error.a str.a fs.a  `cat socket.lib`

ipmeprint.o: \
compile ipmeprint.c subfd.h substdio.h substdio.h ip.h ipme.h ip.h \
ipalloc.h ip.h gen_alloc.h exit.h
	./compile ipmeprint.c

it: \
qmail-local qmail-lspawn qmail-getpw qmail-remote qmail-rspawn \
qmail-clean qmail-send qmail-start splogger qmail-queue qmail-inject \
predate datemail mailsubj qmail-upq qmail-showctl qmail-newu \
qmail-pw2u qmail-qread qmail-qstat qmail-tcpto qmail-tcpok \
qmail-pop3d qmail-popup qmail-qmqpc qmail-qmqpd qmail-qmtpd \
qmail-smtpd sendmail tcp-env qmail-newmrh config config-fast dnscname \
dnsptr dnsip dnsmxip dnsfq hostname ipmeprint qreceipt qsmhook qbiff \
forward preline condredirect bouncesaying except maildirmake \
maildir2mbox maildirwatch qail elq pinq idedit install-big install \
instcheck home home+df proc proc+df binm1 binm1+df binm2 binm2+df \
binm3 binm3+df

load: \
make-load warn-auto.sh systype
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

lock.a: \
makelib lock_ex.o lock_exnb.o lock_un.o
	./makelib lock.a lock_ex.o lock_exnb.o lock_un.o

lock_ex.o: \
compile lock_ex.c hasflock.h lock.h
	./compile lock_ex.c

lock_exnb.o: \
compile lock_exnb.c hasflock.h lock.h
	./compile lock_exnb.c

lock_un.o: \
compile lock_un.c hasflock.h lock.h
	./compile lock_un.c

maildir.0: \
maildir.5
	nroff -man maildir.5 > maildir.0

maildir.o: \
compile maildir.c prioq.h datetime.h gen_alloc.h env.h stralloc.h \
gen_alloc.h direntry.h datetime.h now.h datetime.h str.h maildir.h \
strerr.h
	./compile maildir.c

maildir2mbox: \
load maildir2mbox.o maildir.o prioq.o now.o myctime.o gfrom.o lock.a \
getln.a env.a open.a strerr.a stralloc.a alloc.a substdio.a error.a \
str.a fs.a datetime.a
	./load maildir2mbox maildir.o prioq.o now.o myctime.o \
	gfrom.o lock.a getln.a env.a open.a strerr.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a datetime.a 

maildir2mbox.0: \
maildir2mbox.1
	nroff -man maildir2mbox.1 > maildir2mbox.0

maildir2mbox.o: \
compile maildir2mbox.c readwrite.h prioq.h datetime.h gen_alloc.h \
env.h stralloc.h gen_alloc.h subfd.h substdio.h substdio.h getln.h \
error.h open.h lock.h gfrom.h str.h exit.h myctime.h maildir.h \
strerr.h
	./compile maildir2mbox.c

maildirmake: \
load maildirmake.o strerr.a substdio.a error.a str.a
	./load maildirmake strerr.a substdio.a error.a str.a 

maildirmake.0: \
maildirmake.1
	nroff -man maildirmake.1 > maildirmake.0

maildirmake.o: \
compile maildirmake.c strerr.h exit.h
	./compile maildirmake.c

maildirwatch: \
load maildirwatch.o hfield.o headerbody.o maildir.o prioq.o now.o \
getln.a env.a open.a strerr.a stralloc.a alloc.a substdio.a error.a \
str.a
	./load maildirwatch hfield.o headerbody.o maildir.o \
	prioq.o now.o getln.a env.a open.a strerr.a stralloc.a \
	alloc.a substdio.a error.a str.a 

maildirwatch.0: \
maildirwatch.1
	nroff -man maildirwatch.1 > maildirwatch.0

maildirwatch.o: \
compile maildirwatch.c getln.h substdio.h subfd.h substdio.h prioq.h \
datetime.h gen_alloc.h stralloc.h gen_alloc.h str.h exit.h hfield.h \
readwrite.h open.h headerbody.h maildir.h strerr.h
	./compile maildirwatch.c

mailsubj: \
warn-auto.sh mailsubj.sh conf-qmail conf-break conf-split
	cat warn-auto.sh mailsubj.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> mailsubj
	chmod 755 mailsubj

mailsubj.0: \
mailsubj.1
	nroff -man mailsubj.1 > mailsubj.0

make-compile: \
make-compile.sh auto-ccld.sh
	cat auto-ccld.sh make-compile.sh > make-compile
	chmod 755 make-compile

make-load: \
make-load.sh auto-ccld.sh
	cat auto-ccld.sh make-load.sh > make-load
	chmod 755 make-load

make-makelib: \
make-makelib.sh auto-ccld.sh
	cat auto-ccld.sh make-makelib.sh > make-makelib
	chmod 755 make-makelib

makelib: \
make-makelib warn-auto.sh systype
	( cat warn-auto.sh; ./make-makelib "`cat systype`" ) > \
	makelib
	chmod 755 makelib

man: \
qmail-local.0 qmail-lspawn.0 qmail-getpw.0 qmail-remote.0 \
qmail-rspawn.0 qmail-clean.0 qmail-send.0 qmail-start.0 splogger.0 \
qmail-queue.0 qmail-inject.0 mailsubj.0 qmail-showctl.0 qmail-newu.0 \
qmail-pw2u.0 qmail-qread.0 qmail-qstat.0 qmail-tcpto.0 qmail-tcpok.0 \
qmail-pop3d.0 qmail-popup.0 qmail-qmqpc.0 qmail-qmqpd.0 qmail-qmtpd.0 \
qmail-smtpd.0 tcp-env.0 qmail-newmrh.0 qreceipt.0 qbiff.0 forward.0 \
preline.0 condredirect.0 bouncesaying.0 except.0 maildirmake.0 \
maildir2mbox.0 maildirwatch.0 qmail.0 qmail-limits.0 qmail-log.0 \
qmail-control.0 qmail-header.0 qmail-users.0 dot-qmail.0 \
qmail-command.0 tcp-environ.0 maildir.0 mbox.0 addresses.0 \
envelopes.0 forgeries.0

mbox.0: \
mbox.5
	nroff -man mbox.5 > mbox.0

myctime.o: \
compile myctime.c datetime.h fmt.h myctime.h
	./compile myctime.c

ndelay.a: \
makelib ndelay.o ndelay_off.o
	./makelib ndelay.a ndelay.o ndelay_off.o

ndelay.o: \
compile ndelay.c ndelay.h
	./compile ndelay.c

ndelay_off.o: \
compile ndelay_off.c ndelay.h
	./compile ndelay_off.c

newfield.o: \
compile newfield.c fmt.h datetime.h stralloc.h gen_alloc.h \
date822fmt.h newfield.h stralloc.h
	./compile newfield.c

now.o: \
compile now.c datetime.h now.h datetime.h
	./compile now.c

open.a: \
makelib open_append.o open_excl.o open_read.o open_trunc.o \
open_write.o
	./makelib open.a open_append.o open_excl.o open_read.o \
	open_trunc.o open_write.o

open_append.o: \
compile open_append.c open.h
	./compile open_append.c

open_excl.o: \
compile open_excl.c open.h
	./compile open_excl.c

open_read.o: \
compile open_read.c open.h
	./compile open_read.c

open_trunc.o: \
compile open_trunc.c open.h
	./compile open_trunc.c

open_write.o: \
compile open_write.c open.h
	./compile open_write.c

pinq: \
warn-auto.sh pinq.sh conf-qmail conf-break conf-split
	cat warn-auto.sh pinq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> pinq
	chmod 755 pinq

predate: \
load predate.o datetime.a strerr.a sig.a fd.a wait.a substdio.a \
error.a str.a fs.a
	./load predate datetime.a strerr.a sig.a fd.a wait.a \
	substdio.a error.a str.a fs.a 

predate.o: \
compile predate.c datetime.h fork.h wait.h fd.h fmt.h strerr.h \
substdio.h subfd.h substdio.h readwrite.h exit.h
	./compile predate.c

preline: \
load preline.o strerr.a fd.a wait.a sig.a env.a getopt.a substdio.a \
error.a str.a
	./load preline strerr.a fd.a wait.a sig.a env.a getopt.a \
	substdio.a error.a str.a 

preline.0: \
preline.1
	nroff -man preline.1 > preline.0

preline.o: \
compile preline.c fd.h sgetopt.h subgetopt.h readwrite.h strerr.h \
substdio.h exit.h fork.h wait.h env.h sig.h error.h
	./compile preline.c

prioq.o: \
compile prioq.c alloc.h gen_allocdefs.h prioq.h datetime.h \
gen_alloc.h
	./compile prioq.c

proc: \
proc.sh conf-qmail
	cat proc.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> proc
	chmod 755 proc

proc+df: \
proc+df.sh conf-qmail
	cat proc+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> proc+df
	chmod 755 proc+df

prot.o: \
compile prot.c hasshsgr.h prot.h
	./compile prot.c

qail: \
warn-auto.sh qail.sh conf-qmail conf-break conf-split
	cat warn-auto.sh qail.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qail
	chmod 755 qail

qbiff: \
load qbiff.o headerbody.o hfield.o getln.a env.a open.a stralloc.a \
alloc.a substdio.a error.a str.a
	./load qbiff headerbody.o hfield.o getln.a env.a open.a \
	stralloc.a alloc.a substdio.a error.a str.a 

qbiff.0: \
qbiff.1
	nroff -man qbiff.1 > qbiff.0

qbiff.o: \
compile qbiff.c readwrite.h stralloc.h gen_alloc.h substdio.h subfd.h \
substdio.h open.h byte.h str.h headerbody.h hfield.h env.h exit.h
	./compile qbiff.c

qmail-clean: \
load qmail-clean.o fmtqfn.o now.o getln.a sig.a stralloc.a alloc.a \
substdio.a error.a str.a fs.a auto_qmail.o auto_split.o
	./load qmail-clean fmtqfn.o now.o getln.a sig.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o \
	auto_split.o 

qmail-clean.0: \
qmail-clean.8
	nroff -man qmail-clean.8 > qmail-clean.0

qmail-clean.o: \
compile qmail-clean.c readwrite.h sig.h now.h datetime.h str.h \
direntry.h getln.h stralloc.h gen_alloc.h substdio.h subfd.h \
substdio.h byte.h scan.h fmt.h error.h exit.h fmtqfn.h auto_qmail.h
	./compile qmail-clean.c

qmail-command.0: \
qmail-command.8
	nroff -man qmail-command.8 > qmail-command.0

qmail-control.0: \
qmail-control.5
	nroff -man qmail-control.5 > qmail-control.0

qmail-control.5: \
qmail-control.9 conf-break conf-spawn
	cat qmail-control.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-control.5

qmail-getpw: \
load qmail-getpw.o case.a substdio.a error.a str.a fs.a auto_break.o \
auto_usera.o
	./load qmail-getpw case.a substdio.a error.a str.a fs.a \
	auto_break.o auto_usera.o 

qmail-getpw.0: \
qmail-getpw.8
	nroff -man qmail-getpw.8 > qmail-getpw.0

qmail-getpw.8: \
qmail-getpw.9 conf-break conf-spawn
	cat qmail-getpw.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-getpw.8

qmail-getpw.o: \
compile qmail-getpw.c readwrite.h substdio.h subfd.h substdio.h \
error.h exit.h byte.h str.h case.h fmt.h auto_usera.h auto_break.h \
qlx.h
	./compile qmail-getpw.c

qmail-header.0: \
qmail-header.5
	nroff -man qmail-header.5 > qmail-header.0

qmail-inject: \
load qmail-inject.o headerbody.o hfield.o newfield.o quote.o now.o \
control.o date822fmt.o constmap.o qmail.o case.a fd.a wait.a open.a \
getln.a sig.a getopt.a datetime.a token822.o env.a stralloc.a alloc.a \
substdio.a error.a str.a fs.a auto_qmail.o
	./load qmail-inject headerbody.o hfield.o newfield.o \
	quote.o now.o control.o date822fmt.o constmap.o qmail.o \
	case.a fd.a wait.a open.a getln.a sig.a getopt.a datetime.a \
	token822.o env.a stralloc.a alloc.a substdio.a error.a \
	str.a fs.a auto_qmail.o 

qmail-inject.0: \
qmail-inject.8
	nroff -man qmail-inject.8 > qmail-inject.0

qmail-inject.o: \
compile qmail-inject.c sig.h substdio.h stralloc.h gen_alloc.h \
subfd.h substdio.h sgetopt.h subgetopt.h getln.h alloc.h str.h fmt.h \
hfield.h token822.h gen_alloc.h control.h env.h gen_alloc.h \
gen_allocdefs.h error.h qmail.h substdio.h now.h datetime.h exit.h \
quote.h headerbody.h auto_qmail.h newfield.h stralloc.h constmap.h
	./compile qmail-inject.c

qmail-limits.0: \
qmail-limits.7
	nroff -man qmail-limits.7 > qmail-limits.0

qmail-limits.7: \
qmail-limits.9 conf-break conf-spawn
	cat qmail-limits.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-limits.7

qmail-local: \
load qmail-local.o qmail.o quote.o now.o gfrom.o myctime.o \
slurpclose.o case.a getln.a getopt.a sig.a open.a seek.a lock.a fd.a \
wait.a env.a stralloc.a alloc.a strerr.a substdio.a error.a str.a \
fs.a datetime.a auto_qmail.o auto_patrn.o socket.lib
	./load qmail-local qmail.o quote.o now.o gfrom.o myctime.o \
	slurpclose.o case.a getln.a getopt.a sig.a open.a seek.a \
	lock.a fd.a wait.a env.a stralloc.a alloc.a strerr.a \
	substdio.a error.a str.a fs.a datetime.a auto_qmail.o \
	auto_patrn.o  `cat socket.lib`

qmail-local.0: \
qmail-local.8
	nroff -man qmail-local.8 > qmail-local.0

qmail-local.o: \
compile qmail-local.c readwrite.h sig.h env.h byte.h exit.h fork.h \
open.h wait.h lock.h seek.h substdio.h getln.h strerr.h subfd.h \
substdio.h sgetopt.h subgetopt.h alloc.h error.h stralloc.h \
gen_alloc.h fmt.h str.h now.h datetime.h case.h quote.h qmail.h \
substdio.h slurpclose.h myctime.h gfrom.h auto_patrn.h
	./compile qmail-local.c

qmail-log.0: \
qmail-log.5
	nroff -man qmail-log.5 > qmail-log.0

qmail-lspawn: \
load qmail-lspawn.o spawn.o prot.o slurpclose.o coe.o sig.a wait.a \
case.a cdb.a fd.a open.a stralloc.a alloc.a substdio.a error.a str.a \
fs.a auto_qmail.o auto_uids.o auto_spawn.o
	./load qmail-lspawn spawn.o prot.o slurpclose.o coe.o \
	sig.a wait.a case.a cdb.a fd.a open.a stralloc.a alloc.a \
	substdio.a error.a str.a fs.a auto_qmail.o auto_uids.o \
	auto_spawn.o 

qmail-lspawn.0: \
qmail-lspawn.8
	nroff -man qmail-lspawn.8 > qmail-lspawn.0

qmail-lspawn.o: \
compile qmail-lspawn.c fd.h wait.h prot.h substdio.h stralloc.h \
gen_alloc.h scan.h exit.h fork.h error.h cdb.h uint32.h case.h \
slurpclose.h auto_qmail.h auto_uids.h qlx.h
	./compile qmail-lspawn.c

qmail-newmrh: \
load qmail-newmrh.o cdbmss.o getln.a open.a cdbmake.a seek.a case.a \
stralloc.a alloc.a strerr.a substdio.a error.a str.a auto_qmail.o
	./load qmail-newmrh cdbmss.o getln.a open.a cdbmake.a \
	seek.a case.a stralloc.a alloc.a strerr.a substdio.a \
	error.a str.a auto_qmail.o 

qmail-newmrh.0: \
qmail-newmrh.8
	nroff -man qmail-newmrh.8 > qmail-newmrh.0

qmail-newmrh.8: \
qmail-newmrh.9 conf-break conf-spawn
	cat qmail-newmrh.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-newmrh.8

qmail-newmrh.o: \
compile qmail-newmrh.c strerr.h stralloc.h gen_alloc.h substdio.h \
getln.h exit.h readwrite.h open.h auto_qmail.h cdbmss.h cdbmake.h \
uint32.h substdio.h
	./compile qmail-newmrh.c

qmail-newu: \
load qmail-newu.o cdbmss.o getln.a open.a seek.a cdbmake.a case.a \
stralloc.a alloc.a substdio.a error.a str.a auto_qmail.o
	./load qmail-newu cdbmss.o getln.a open.a seek.a cdbmake.a \
	case.a stralloc.a alloc.a substdio.a error.a str.a \
	auto_qmail.o 

qmail-newu.0: \
qmail-newu.8
	nroff -man qmail-newu.8 > qmail-newu.0

qmail-newu.8: \
qmail-newu.9 conf-break conf-spawn
	cat qmail-newu.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-newu.8

qmail-newu.o: \
compile qmail-newu.c stralloc.h gen_alloc.h subfd.h substdio.h \
getln.h substdio.h cdbmss.h cdbmake.h uint32.h substdio.h exit.h \
readwrite.h open.h error.h case.h auto_qmail.h
	./compile qmail-newu.c

qmail-pop3d: \
load qmail-pop3d.o commands.o case.a timeoutread.o timeoutwrite.o \
maildir.o prioq.o now.o env.a strerr.a sig.a open.a getln.a \
stralloc.a alloc.a substdio.a error.a str.a fs.a socket.lib
	./load qmail-pop3d commands.o case.a timeoutread.o \
	timeoutwrite.o maildir.o prioq.o now.o env.a strerr.a sig.a \
	open.a getln.a stralloc.a alloc.a substdio.a error.a str.a \
	fs.a  `cat socket.lib`

qmail-pop3d.0: \
qmail-pop3d.8
	nroff -man qmail-pop3d.8 > qmail-pop3d.0

qmail-pop3d.o: \
compile qmail-pop3d.c commands.h sig.h getln.h stralloc.h gen_alloc.h \
substdio.h alloc.h open.h prioq.h datetime.h gen_alloc.h scan.h fmt.h \
str.h exit.h maildir.h strerr.h readwrite.h timeoutread.h \
timeoutwrite.h
	./compile qmail-pop3d.c

qmail-popup: \
load qmail-popup.o commands.o timeoutread.o timeoutwrite.o now.o \
case.a fd.a sig.a wait.a stralloc.a alloc.a substdio.a error.a str.a \
fs.a socket.lib
	./load qmail-popup commands.o timeoutread.o timeoutwrite.o \
	now.o case.a fd.a sig.a wait.a stralloc.a alloc.a \
	substdio.a error.a str.a fs.a  `cat socket.lib`

qmail-popup.0: \
qmail-popup.8
	nroff -man qmail-popup.8 > qmail-popup.0

qmail-popup.o: \
compile qmail-popup.c commands.h fd.h sig.h stralloc.h gen_alloc.h \
substdio.h alloc.h wait.h str.h byte.h now.h datetime.h fmt.h exit.h \
readwrite.h timeoutread.h timeoutwrite.h
	./compile qmail-popup.c

qmail-pw2u: \
load qmail-pw2u.o constmap.o control.o open.a getln.a case.a getopt.a \
stralloc.a alloc.a substdio.a error.a str.a fs.a auto_usera.o \
auto_break.o auto_qmail.o
	./load qmail-pw2u constmap.o control.o open.a getln.a \
	case.a getopt.a stralloc.a alloc.a substdio.a error.a str.a \
	fs.a auto_usera.o auto_break.o auto_qmail.o 

qmail-pw2u.0: \
qmail-pw2u.8
	nroff -man qmail-pw2u.8 > qmail-pw2u.0

qmail-pw2u.8: \
qmail-pw2u.9 conf-break conf-spawn
	cat qmail-pw2u.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-pw2u.8

qmail-pw2u.o: \
compile qmail-pw2u.c substdio.h readwrite.h subfd.h substdio.h \
sgetopt.h subgetopt.h control.h constmap.h stralloc.h gen_alloc.h \
fmt.h str.h scan.h open.h error.h getln.h auto_break.h auto_qmail.h \
auto_usera.h
	./compile qmail-pw2u.c

qmail-qmqpc: \
load qmail-qmqpc.o slurpclose.o timeoutread.o timeoutwrite.o \
timeoutconn.o ip.o control.o auto_qmail.o sig.a ndelay.a open.a \
getln.a substdio.a stralloc.a alloc.a error.a str.a fs.a socket.lib
	./load qmail-qmqpc slurpclose.o timeoutread.o \
	timeoutwrite.o timeoutconn.o ip.o control.o auto_qmail.o \
	sig.a ndelay.a open.a getln.a substdio.a stralloc.a alloc.a \
	error.a str.a fs.a  `cat socket.lib`

qmail-qmqpc.0: \
qmail-qmqpc.8
	nroff -man qmail-qmqpc.8 > qmail-qmqpc.0

qmail-qmqpc.o: \
compile qmail-qmqpc.c substdio.h getln.h readwrite.h exit.h \
stralloc.h gen_alloc.h slurpclose.h error.h sig.h ip.h timeoutconn.h \
timeoutread.h timeoutwrite.h auto_qmail.h control.h fmt.h
	./compile qmail-qmqpc.c

qmail-qmqpd: \
load qmail-qmqpd.o received.o now.o date822fmt.o qmail.o auto_qmail.o \
env.a substdio.a sig.a error.a wait.a fd.a str.a datetime.a fs.a
	./load qmail-qmqpd received.o now.o date822fmt.o qmail.o \
	auto_qmail.o env.a substdio.a sig.a error.a wait.a fd.a \
	str.a datetime.a fs.a 

qmail-qmqpd.0: \
qmail-qmqpd.8
	nroff -man qmail-qmqpd.8 > qmail-qmqpd.0

qmail-qmqpd.o: \
compile qmail-qmqpd.c auto_qmail.h qmail.h substdio.h received.h \
sig.h substdio.h readwrite.h exit.h now.h datetime.h fmt.h env.h
	./compile qmail-qmqpd.c

qmail-qmtpd: \
load qmail-qmtpd.o rcpthosts.o control.o constmap.o received.o \
date822fmt.o now.o qmail.o cdb.a fd.a wait.a datetime.a open.a \
getln.a sig.a case.a env.a stralloc.a alloc.a substdio.a error.a \
str.a fs.a auto_qmail.o
	./load qmail-qmtpd rcpthosts.o control.o constmap.o \
	received.o date822fmt.o now.o qmail.o cdb.a fd.a wait.a \
	datetime.a open.a getln.a sig.a case.a env.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o 

qmail-qmtpd.0: \
qmail-qmtpd.8
	nroff -man qmail-qmtpd.8 > qmail-qmtpd.0

qmail-qmtpd.o: \
compile qmail-qmtpd.c stralloc.h gen_alloc.h substdio.h qmail.h \
substdio.h now.h datetime.h str.h fmt.h env.h sig.h rcpthosts.h \
auto_qmail.h readwrite.h control.h received.h
	./compile qmail-qmtpd.c

qmail-qread: \
load qmail-qread.o fmtqfn.o readsubdir.o date822fmt.o datetime.a \
open.a getln.a stralloc.a alloc.a substdio.a error.a str.a fs.a \
auto_qmail.o auto_split.o
	./load qmail-qread fmtqfn.o readsubdir.o date822fmt.o \
	datetime.a open.a getln.a stralloc.a alloc.a substdio.a \
	error.a str.a fs.a auto_qmail.o auto_split.o 

qmail-qread.0: \
qmail-qread.8
	nroff -man qmail-qread.8 > qmail-qread.0

qmail-qread.o: \
compile qmail-qread.c stralloc.h gen_alloc.h substdio.h subfd.h \
substdio.h fmt.h str.h getln.h fmtqfn.h readsubdir.h direntry.h \
auto_qmail.h open.h datetime.h date822fmt.h readwrite.h error.h \
exit.h
	./compile qmail-qread.c

qmail-qstat: \
warn-auto.sh qmail-qstat.sh conf-qmail conf-break conf-split
	cat warn-auto.sh qmail-qstat.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qmail-qstat
	chmod 755 qmail-qstat

qmail-qstat.0: \
qmail-qstat.8
	nroff -man qmail-qstat.8 > qmail-qstat.0

qmail-queue: \
load qmail-queue.o triggerpull.o fmtqfn.o now.o date822fmt.o \
datetime.a seek.a ndelay.a open.a sig.a alloc.a substdio.a error.a \
str.a fs.a auto_qmail.o auto_split.o auto_uids.o
	./load qmail-queue triggerpull.o fmtqfn.o now.o \
	date822fmt.o datetime.a seek.a ndelay.a open.a sig.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o \
	auto_split.o auto_uids.o 

qmail-queue.0: \
qmail-queue.8
	nroff -man qmail-queue.8 > qmail-queue.0

qmail-queue.o: \
compile qmail-queue.c readwrite.h sig.h exit.h open.h seek.h fmt.h \
alloc.h substdio.h datetime.h now.h datetime.h triggerpull.h extra.h \
auto_qmail.h auto_uids.h date822fmt.h fmtqfn.h
	./compile qmail-queue.c

qmail-remote: \
load qmail-remote.o control.o constmap.o timeoutread.o timeoutwrite.o \
timeoutconn.o tcpto.o now.o dns.o ip.o ipalloc.o ipme.o quote.o \
ndelay.a case.a sig.a open.a lock.a seek.a getln.a stralloc.a alloc.a \
substdio.a error.a str.a fs.a auto_qmail.o dns.lib socket.lib
	./load qmail-remote control.o constmap.o timeoutread.o \
	timeoutwrite.o timeoutconn.o tcpto.o now.o dns.o ip.o \
	ipalloc.o ipme.o quote.o ndelay.a case.a sig.a open.a \
	lock.a seek.a getln.a stralloc.a alloc.a substdio.a error.a \
	str.a fs.a auto_qmail.o  `cat dns.lib` `cat socket.lib`

qmail-remote.0: \
qmail-remote.8
	nroff -man qmail-remote.8 > qmail-remote.0

qmail-remote.o: \
compile qmail-remote.c sig.h stralloc.h gen_alloc.h substdio.h \
subfd.h substdio.h scan.h case.h error.h auto_qmail.h control.h dns.h \
alloc.h quote.h ip.h ipalloc.h ip.h gen_alloc.h ipme.h ip.h ipalloc.h \
gen_alloc.h gen_allocdefs.h str.h now.h datetime.h exit.h constmap.h \
tcpto.h readwrite.h timeoutconn.h timeoutread.h timeoutwrite.h
	./compile qmail-remote.c

qmail-rspawn: \
load qmail-rspawn.o spawn.o tcpto_clean.o now.o coe.o sig.a open.a \
seek.a lock.a wait.a fd.a stralloc.a alloc.a substdio.a error.a str.a \
auto_qmail.o auto_uids.o auto_spawn.o
	./load qmail-rspawn spawn.o tcpto_clean.o now.o coe.o \
	sig.a open.a seek.a lock.a wait.a fd.a stralloc.a alloc.a \
	substdio.a error.a str.a auto_qmail.o auto_uids.o \
	auto_spawn.o 

qmail-rspawn.0: \
qmail-rspawn.8
	nroff -man qmail-rspawn.8 > qmail-rspawn.0

qmail-rspawn.o: \
compile qmail-rspawn.c fd.h wait.h substdio.h exit.h fork.h error.h \
tcpto.h
	./compile qmail-rspawn.c

qmail-send: \
load qmail-send.o qsutil.o control.o constmap.o newfield.o prioq.o \
trigger.o fmtqfn.o quote.o now.o readsubdir.o qmail.o date822fmt.o \
datetime.a case.a ndelay.a getln.a wait.a seek.a fd.a sig.a open.a \
lock.a stralloc.a alloc.a substdio.a error.a str.a fs.a auto_qmail.o \
auto_split.o
	./load qmail-send qsutil.o control.o constmap.o newfield.o \
	prioq.o trigger.o fmtqfn.o quote.o now.o readsubdir.o \
	qmail.o date822fmt.o datetime.a case.a ndelay.a getln.a \
	wait.a seek.a fd.a sig.a open.a lock.a stralloc.a alloc.a \
	substdio.a error.a str.a fs.a auto_qmail.o auto_split.o 

qmail-send.0: \
qmail-send.8
	nroff -man qmail-send.8 > qmail-send.0

qmail-send.8: \
qmail-send.9 conf-break conf-spawn
	cat qmail-send.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-send.8

qmail-send.o: \
compile qmail-send.c readwrite.h sig.h direntry.h control.h select.h \
open.h seek.h exit.h lock.h ndelay.h now.h datetime.h getln.h \
substdio.h alloc.h error.h stralloc.h gen_alloc.h str.h byte.h fmt.h \
scan.h case.h auto_qmail.h trigger.h newfield.h stralloc.h quote.h \
qmail.h substdio.h qsutil.h prioq.h datetime.h gen_alloc.h constmap.h \
fmtqfn.h readsubdir.h direntry.h
	./compile qmail-send.c

qmail-showctl: \
load qmail-showctl.o auto_uids.o control.o open.a getln.a stralloc.a \
alloc.a substdio.a error.a str.a fs.a auto_qmail.o auto_break.o \
auto_patrn.o auto_spawn.o auto_split.o
	./load qmail-showctl auto_uids.o control.o open.a getln.a \
	stralloc.a alloc.a substdio.a error.a str.a fs.a \
	auto_qmail.o auto_break.o auto_patrn.o auto_spawn.o \
	auto_split.o 

qmail-showctl.0: \
qmail-showctl.8
	nroff -man qmail-showctl.8 > qmail-showctl.0

qmail-showctl.o: \
compile qmail-showctl.c substdio.h subfd.h substdio.h exit.h fmt.h \
str.h control.h constmap.h stralloc.h gen_alloc.h direntry.h \
auto_uids.h auto_qmail.h auto_break.h auto_patrn.h auto_spawn.h \
auto_split.h
	./compile qmail-showctl.c

qmail-smtpd: \
load qmail-smtpd.o rcpthosts.o commands.o timeoutread.o \
timeoutwrite.o ip.o ipme.o ipalloc.o control.o constmap.o received.o \
date822fmt.o now.o qmail.o cdb.a fd.a wait.a datetime.a getln.a \
open.a sig.a case.a env.a stralloc.a alloc.a substdio.a error.a str.a \
fs.a auto_qmail.o socket.lib
	./load qmail-smtpd rcpthosts.o commands.o timeoutread.o \
	timeoutwrite.o ip.o ipme.o ipalloc.o control.o constmap.o \
	received.o date822fmt.o now.o qmail.o cdb.a fd.a wait.a \
	datetime.a getln.a open.a sig.a case.a env.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o  `cat \
	socket.lib`

qmail-smtpd.0: \
qmail-smtpd.8
	nroff -man qmail-smtpd.8 > qmail-smtpd.0

qmail-smtpd.o: \
compile qmail-smtpd.c sig.h readwrite.h stralloc.h gen_alloc.h \
substdio.h alloc.h auto_qmail.h control.h received.h constmap.h \
error.h ipme.h ip.h ipalloc.h ip.h gen_alloc.h ip.h qmail.h \
substdio.h str.h fmt.h scan.h byte.h case.h env.h now.h datetime.h \
exit.h rcpthosts.h timeoutread.h timeoutwrite.h commands.h
	./compile qmail-smtpd.c

qmail-start: \
load qmail-start.o prot.o fd.a auto_uids.o
	./load qmail-start prot.o fd.a auto_uids.o 

qmail-start.0: \
qmail-start.8
	nroff -man qmail-start.8 > qmail-start.0

qmail-start.8: \
qmail-start.9 conf-break conf-spawn
	cat qmail-start.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-start.8

qmail-start.o: \
compile qmail-start.c fd.h prot.h exit.h fork.h auto_uids.h
	./compile qmail-start.c

qmail-tcpok: \
load qmail-tcpok.o open.a lock.a strerr.a substdio.a error.a str.a \
auto_qmail.o
	./load qmail-tcpok open.a lock.a strerr.a substdio.a \
	error.a str.a auto_qmail.o 

qmail-tcpok.0: \
qmail-tcpok.8
	nroff -man qmail-tcpok.8 > qmail-tcpok.0

qmail-tcpok.o: \
compile qmail-tcpok.c strerr.h substdio.h lock.h open.h readwrite.h \
auto_qmail.h exit.h
	./compile qmail-tcpok.c

qmail-tcpto: \
load qmail-tcpto.o ip.o now.o open.a lock.a substdio.a error.a str.a \
fs.a auto_qmail.o
	./load qmail-tcpto ip.o now.o open.a lock.a substdio.a \
	error.a str.a fs.a auto_qmail.o 

qmail-tcpto.0: \
qmail-tcpto.8
	nroff -man qmail-tcpto.8 > qmail-tcpto.0

qmail-tcpto.o: \
compile qmail-tcpto.c substdio.h subfd.h substdio.h auto_qmail.h \
fmt.h ip.h lock.h error.h exit.h datetime.h now.h datetime.h
	./compile qmail-tcpto.c

qmail-upq: \
warn-auto.sh qmail-upq.sh conf-qmail conf-break conf-split
	cat warn-auto.sh qmail-upq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qmail-upq
	chmod 755 qmail-upq

qmail-users.0: \
qmail-users.5
	nroff -man qmail-users.5 > qmail-users.0

qmail-users.5: \
qmail-users.9 conf-break conf-spawn
	cat qmail-users.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-users.5

qmail.0: \
qmail.7
	nroff -man qmail.7 > qmail.0

qmail.o: \
compile qmail.c substdio.h readwrite.h wait.h exit.h fork.h fd.h \
qmail.h substdio.h auto_qmail.h
	./compile qmail.c

qreceipt: \
load qreceipt.o headerbody.o hfield.o quote.o token822.o qmail.o \
getln.a fd.a wait.a sig.a env.a stralloc.a alloc.a substdio.a error.a \
str.a auto_qmail.o
	./load qreceipt headerbody.o hfield.o quote.o token822.o \
	qmail.o getln.a fd.a wait.a sig.a env.a stralloc.a alloc.a \
	substdio.a error.a str.a auto_qmail.o 

qreceipt.0: \
qreceipt.1
	nroff -man qreceipt.1 > qreceipt.0

qreceipt.o: \
compile qreceipt.c sig.h env.h substdio.h stralloc.h gen_alloc.h \
subfd.h substdio.h getln.h alloc.h str.h hfield.h token822.h \
gen_alloc.h error.h gen_alloc.h gen_allocdefs.h headerbody.h exit.h \
open.h quote.h qmail.h substdio.h
	./compile qreceipt.c

qsmhook: \
load qsmhook.o sig.a case.a fd.a wait.a getopt.a env.a stralloc.a \
alloc.a substdio.a error.a str.a
	./load qsmhook sig.a case.a fd.a wait.a getopt.a env.a \
	stralloc.a alloc.a substdio.a error.a str.a 

qsmhook.o: \
compile qsmhook.c fd.h stralloc.h gen_alloc.h readwrite.h sgetopt.h \
subgetopt.h wait.h env.h byte.h str.h alloc.h exit.h fork.h case.h \
subfd.h substdio.h error.h substdio.h sig.h
	./compile qsmhook.c

qsutil.o: \
compile qsutil.c stralloc.h gen_alloc.h readwrite.h substdio.h \
qsutil.h
	./compile qsutil.c

quote.o: \
compile quote.c stralloc.h gen_alloc.h str.h quote.h
	./compile quote.c

rcpthosts.o: \
compile rcpthosts.c cdb.h uint32.h byte.h open.h error.h control.h \
constmap.h stralloc.h gen_alloc.h rcpthosts.h
	./compile rcpthosts.c

readsubdir.o: \
compile readsubdir.c readsubdir.h direntry.h fmt.h scan.h str.h \
auto_split.h
	./compile readsubdir.c

received.o: \
compile received.c fmt.h qmail.h substdio.h now.h datetime.h \
datetime.h date822fmt.h received.h
	./compile received.c

remoteinfo.o: \
compile remoteinfo.c byte.h substdio.h ip.h fmt.h timeoutconn.h \
timeoutread.h timeoutwrite.h remoteinfo.h
	./compile remoteinfo.c

scan_8long.o: \
compile scan_8long.c scan.h
	./compile scan_8long.c

scan_ulong.o: \
compile scan_ulong.c scan.h
	./compile scan_ulong.c

seek.a: \
makelib seek_cur.o seek_end.o seek_set.o seek_trunc.o
	./makelib seek.a seek_cur.o seek_end.o seek_set.o \
	seek_trunc.o

seek_cur.o: \
compile seek_cur.c seek.h
	./compile seek_cur.c

seek_end.o: \
compile seek_end.c seek.h
	./compile seek_end.c

seek_set.o: \
compile seek_set.c seek.h
	./compile seek_set.c

seek_trunc.o: \
compile seek_trunc.c seek.h
	./compile seek_trunc.c

select.h: \
compile trysysel.c select.h1 select.h2
	( ./compile trysysel.c >/dev/null 2>&1 \
	&& cat select.h2 || cat select.h1 ) > select.h
	rm -f trysysel.o trysysel

sendmail: \
load sendmail.o env.a getopt.a alloc.a substdio.a error.a str.a \
auto_qmail.o
	./load sendmail env.a getopt.a alloc.a substdio.a error.a \
	str.a auto_qmail.o 

sendmail.o: \
compile sendmail.c sgetopt.h subgetopt.h substdio.h subfd.h \
substdio.h alloc.h auto_qmail.h exit.h env.h str.h
	./compile sendmail.c

setup: \
it man
	./install

sgetopt.o: \
compile sgetopt.c substdio.h subfd.h substdio.h sgetopt.h subgetopt.h \
subgetopt.h
	./compile sgetopt.c

shar: \
FILES BLURB BLURB2 BLURB3 BLURB4 README FAQ INSTALL INSTALL.alias \
INSTALL.ctl INSTALL.ids INSTALL.maildir INSTALL.mbox INSTALL.vsm \
REMOVE.sendmail REMOVE.binmail TEST.deliver TEST.receive UPGRADE \
THOUGHTS TODO THANKS CHANGES SECURITY INTERNALS SENDMAIL \
PIC.local2alias PIC.local2ext PIC.local2local PIC.local2rem \
PIC.local2virt PIC.nullclient PIC.relaybad PIC.relaygood \
PIC.rem2local FILES VERSION SYSDEPS TARGETS Makefile BIN.README \
BIN.Makefile BIN.setup idedit.c conf-break auto_break.h conf-spawn \
auto_spawn.h chkspawn.c conf-split auto_split.h conf-patrn \
auto_patrn.h conf-users conf-groups auto_uids.h auto_usera.h extra.h \
addresses.5 except.1 bouncesaying.1 condredirect.1 dot-qmail.9 \
envelopes.5 forgeries.7 forward.1 maildir2mbox.1 maildirmake.1 \
maildirwatch.1 mailsubj.1 mbox.5 preline.1 qbiff.1 qmail-clean.8 \
qmail-command.8 qmail-control.9 qmail-getpw.9 qmail-header.5 \
qmail-inject.8 qmail-limits.9 qmail-local.8 qmail-log.5 \
qmail-lspawn.8 qmail-newmrh.9 qmail-newu.9 qmail-pop3d.8 \
qmail-popup.8 qmail-pw2u.9 qmail-qmqpc.8 qmail-qmqpd.8 qmail-qmtpd.8 \
qmail-qread.8 qmail-qstat.8 qmail-queue.8 qmail-remote.8 \
qmail-rspawn.8 qmail-send.9 qmail-showctl.8 qmail-smtpd.8 \
qmail-start.9 qmail-tcpok.8 qmail-tcpto.8 qmail-users.9 qmail.7 \
qreceipt.1 splogger.8 tcp-env.1 config.sh config-fast.sh \
qmail-clean.c qmail-getpw.c qmail-inject.c qmail-local.c \
qmail-lspawn.c qmail-newmrh.c qmail-newu.c qmail-pop3d.c \
qmail-popup.c qmail-pw2u.c qmail-qmqpc.c qmail-qmqpd.c qmail-qmtpd.c \
qmail-qread.c qmail-qstat.sh qmail-queue.c qmail-remote.c \
qmail-rspawn.c qmail-send.c qmail-showctl.c qmail-smtpd.c \
qmail-start.c qmail-tcpok.c qmail-tcpto.c spawn.c dnscname.c dnsfq.c \
dnsip.c dnsmxip.c dnsptr.c hostname.c ipmeprint.c tcp-env.c \
sendmail.c qreceipt.c qsmhook.c qbiff.c forward.c preline.c predate.c \
except.c bouncesaying.c condredirect.c maildirmake.c maildir2mbox.c \
maildirwatch.c splogger.c qail.sh elq.sh pinq.sh qmail-upq.sh \
datemail.sh mailsubj.sh qlx.h rcpthosts.h rcpthosts.c commands.h \
commands.c dnsdoe.h dnsdoe.c fmtqfn.h fmtqfn.c gfrom.h gfrom.c \
myctime.h myctime.c newfield.h newfield.c qsutil.h qsutil.c \
readsubdir.h readsubdir.c received.h received.c tcpto.h tcpto.c \
tcpto_clean.c trigger.h trigger.c triggerpull.h triggerpull.c \
trynpbg1.c trysyslog.c conf-cc conf-ld home.sh home+df.sh proc.sh \
proc+df.sh binm1.sh binm2.sh binm3.sh binm1+df.sh binm2+df.sh \
binm3+df.sh find-systype.sh make-compile.sh make-load.sh \
make-makelib.sh trycpp.c warn-auto.sh auto-str.c auto-int.c \
auto-int8.c auto-gid.c auto-uid.c hier.c install.c instcheck.c \
install-big.c alloc.3 alloc.h alloc.c alloc_re.c case.3 case.h \
case_diffb.c case_diffs.c case_lowerb.c case_lowers.c case_starts.c \
cdb.3 cdb.h cdb_hash.c cdb_seek.c cdb_unpack.c cdbmake.h \
cdbmake_add.c cdbmake_hash.c cdbmake_pack.c cdbmss.h cdbmss.c coe.3 \
coe.h coe.c fd.h fd_copy.3 fd_copy.c fd_move.3 fd_move.c fifo_make.3 \
fifo.h fifo.c trymkffo.c fork.h1 fork.h2 tryvfork.c now.3 now.h now.c \
open.h open_append.c open_excl.c open_read.c open_trunc.c \
open_write.c seek.h seek_cur.c seek_end.c seek_set.c seek_trunc.c \
conf-qmail auto_qmail.h qmail.h qmail.c gen_alloc.h gen_allocdefs.h \
stralloc.3 stralloc.h stralloc_eady.c stralloc_pend.c stralloc_copy.c \
stralloc_opyb.c stralloc_opys.c stralloc_cat.c stralloc_catb.c \
stralloc_cats.c stralloc_arts.c strerr.h strerr_sys.c strerr_die.c \
substdio.h substdio.c substdi.c substdo.c substdio_copy.c subfd.h \
subfderr.c subfdouts.c subfdout.c subfdins.c subfdin.c readwrite.h \
exit.h timeoutconn.h timeoutconn.c timeoutread.h timeoutread.c \
timeoutwrite.h timeoutwrite.c remoteinfo.h remoteinfo.c uint32.h1 \
uint32.h2 tryulong32.c wait.3 wait.h wait_pid.c wait_nohang.c \
trywaitp.c sig.h sig_alarm.c sig_block.c sig_catch.c sig_pause.c \
sig_pipe.c sig_child.c sig_term.c sig_hup.c sig_misc.c sig_bug.c \
trysgact.c trysgprm.c env.3 env.h env.c envread.c byte.h byte_chr.c \
byte_copy.c byte_cr.c byte_diff.c byte_rchr.c byte_zero.c str.h \
str_chr.c str_cpy.c str_diff.c str_diffn.c str_len.c str_rchr.c \
str_start.c lock.h lock_ex.c lock_exnb.c lock_un.c tryflock.c getln.3 \
getln.h getln.c getln2.3 getln2.c sgetopt.3 sgetopt.h sgetopt.c \
subgetopt.3 subgetopt.h subgetopt.c error.3 error_str.3 error_temp.3 \
error.h error.c error_str.c error_temp.c fmt.h fmt_str.c fmt_strn.c \
fmt_uint.c fmt_uint0.c fmt_ulong.c scan.h scan_ulong.c scan_8long.c \
slurpclose.h slurpclose.c quote.h quote.c hfield.h hfield.c \
headerbody.h headerbody.c token822.h token822.c control.h control.c \
datetime.3 datetime.h datetime.c datetime_un.c prioq.h prioq.c \
date822fmt.h date822fmt.c dns.h dns.c trylsock.c tryrsolv.c ip.h ip.c \
ipalloc.h ipalloc.c select.h1 select.h2 trysysel.c ndelay.h ndelay.c \
ndelay_off.c direntry.3 direntry.h1 direntry.h2 trydrent.c prot.h \
prot.c chkshsgr.c warn-shsgr tryshsgr.c ipme.h ipme.c trysalen.c \
maildir.5 maildir.h maildir.c tcp-environ.5 constmap.h constmap.c
	shar -m `cat FILES` > shar
	chmod 400 shar

sig.a: \
makelib sig_alarm.o sig_block.o sig_catch.o sig_pause.o sig_pipe.o \
sig_child.o sig_hup.o sig_term.o sig_bug.o sig_misc.o
	./makelib sig.a sig_alarm.o sig_block.o sig_catch.o \
	sig_pause.o sig_pipe.o sig_child.o sig_hup.o sig_term.o \
	sig_bug.o sig_misc.o

sig_alarm.o: \
compile sig_alarm.c sig.h
	./compile sig_alarm.c

sig_block.o: \
compile sig_block.c sig.h hassgprm.h
	./compile sig_block.c

sig_bug.o: \
compile sig_bug.c sig.h
	./compile sig_bug.c

sig_catch.o: \
compile sig_catch.c sig.h hassgact.h
	./compile sig_catch.c

sig_child.o: \
compile sig_child.c sig.h
	./compile sig_child.c

sig_hup.o: \
compile sig_hup.c sig.h
	./compile sig_hup.c

sig_misc.o: \
compile sig_misc.c sig.h
	./compile sig_misc.c

sig_pause.o: \
compile sig_pause.c sig.h hassgprm.h
	./compile sig_pause.c

sig_pipe.o: \
compile sig_pipe.c sig.h
	./compile sig_pipe.c

sig_term.o: \
compile sig_term.c sig.h
	./compile sig_term.c

slurpclose.o: \
compile slurpclose.c stralloc.h gen_alloc.h readwrite.h slurpclose.h \
error.h
	./compile slurpclose.c

socket.lib: \
trylsock.c compile load
	( ( ./compile trylsock.c && \
	./load trylsock -lsocket -lnsl ) >/dev/null 2>&1 \
	&& echo -lsocket -lnsl || exit 0 ) > socket.lib
	rm -f trylsock.o trylsock

spawn.o: \
compile chkspawn spawn.c sig.h wait.h substdio.h byte.h str.h \
stralloc.h gen_alloc.h select.h exit.h coe.h open.h error.h \
auto_qmail.h auto_uids.h auto_spawn.h
	./chkspawn
	./compile spawn.c

splogger: \
load splogger.o substdio.a error.a str.a fs.a syslog.lib socket.lib
	./load splogger substdio.a error.a str.a fs.a  `cat \
	syslog.lib` `cat socket.lib`

splogger.0: \
splogger.8
	nroff -man splogger.8 > splogger.0

splogger.o: \
compile splogger.c error.h substdio.h subfd.h substdio.h exit.h str.h \
scan.h fmt.h
	./compile splogger.c

str.a: \
makelib str_len.o str_diff.o str_diffn.o str_cpy.o str_chr.o \
str_rchr.o str_start.o byte_chr.o byte_rchr.o byte_diff.o byte_copy.o \
byte_cr.o byte_zero.o
	./makelib str.a str_len.o str_diff.o str_diffn.o str_cpy.o \
	str_chr.o str_rchr.o str_start.o byte_chr.o byte_rchr.o \
	byte_diff.o byte_copy.o byte_cr.o byte_zero.o

str_chr.o: \
compile str_chr.c str.h
	./compile str_chr.c

str_cpy.o: \
compile str_cpy.c str.h
	./compile str_cpy.c

str_diff.o: \
compile str_diff.c str.h
	./compile str_diff.c

str_diffn.o: \
compile str_diffn.c str.h
	./compile str_diffn.c

str_len.o: \
compile str_len.c str.h
	./compile str_len.c

str_rchr.o: \
compile str_rchr.c str.h
	./compile str_rchr.c

str_start.o: \
compile str_start.c str.h
	./compile str_start.c

stralloc.a: \
makelib stralloc_eady.o stralloc_pend.o stralloc_copy.o \
stralloc_opys.o stralloc_opyb.o stralloc_cat.o stralloc_cats.o \
stralloc_catb.o stralloc_arts.o
	./makelib stralloc.a stralloc_eady.o stralloc_pend.o \
	stralloc_copy.o stralloc_opys.o stralloc_opyb.o \
	stralloc_cat.o stralloc_cats.o stralloc_catb.o \
	stralloc_arts.o

stralloc_arts.o: \
compile stralloc_arts.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_arts.c

stralloc_cat.o: \
compile stralloc_cat.c byte.h stralloc.h gen_alloc.h
	./compile stralloc_cat.c

stralloc_catb.o: \
compile stralloc_catb.c stralloc.h gen_alloc.h byte.h
	./compile stralloc_catb.c

stralloc_cats.o: \
compile stralloc_cats.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_cats.c

stralloc_copy.o: \
compile stralloc_copy.c byte.h stralloc.h gen_alloc.h
	./compile stralloc_copy.c

stralloc_eady.o: \
compile stralloc_eady.c alloc.h stralloc.h gen_alloc.h \
gen_allocdefs.h
	./compile stralloc_eady.c

stralloc_opyb.o: \
compile stralloc_opyb.c stralloc.h gen_alloc.h byte.h
	./compile stralloc_opyb.c

stralloc_opys.o: \
compile stralloc_opys.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_opys.c

stralloc_pend.o: \
compile stralloc_pend.c alloc.h stralloc.h gen_alloc.h \
gen_allocdefs.h
	./compile stralloc_pend.c

strerr.a: \
makelib strerr_sys.o strerr_die.o
	./makelib strerr.a strerr_sys.o strerr_die.o

strerr_die.o: \
compile strerr_die.c substdio.h subfd.h substdio.h exit.h strerr.h
	./compile strerr_die.c

strerr_sys.o: \
compile strerr_sys.c error.h strerr.h
	./compile strerr_sys.c

subfderr.o: \
compile subfderr.c readwrite.h substdio.h subfd.h substdio.h
	./compile subfderr.c

subfdin.o: \
compile subfdin.c readwrite.h substdio.h subfd.h substdio.h
	./compile subfdin.c

subfdins.o: \
compile subfdins.c readwrite.h substdio.h subfd.h substdio.h
	./compile subfdins.c

subfdout.o: \
compile subfdout.c readwrite.h substdio.h subfd.h substdio.h
	./compile subfdout.c

subfdouts.o: \
compile subfdouts.c readwrite.h substdio.h subfd.h substdio.h
	./compile subfdouts.c

subgetopt.o: \
compile subgetopt.c subgetopt.h
	./compile subgetopt.c

substdi.o: \
compile substdi.c substdio.h byte.h error.h
	./compile substdi.c

substdio.a: \
makelib substdio.o substdi.o substdo.o subfderr.o subfdout.o \
subfdouts.o subfdin.o subfdins.o substdio_copy.o
	./makelib substdio.a substdio.o substdi.o substdo.o \
	subfderr.o subfdout.o subfdouts.o subfdin.o subfdins.o \
	substdio_copy.o

substdio.o: \
compile substdio.c substdio.h
	./compile substdio.c

substdio_copy.o: \
compile substdio_copy.c substdio.h
	./compile substdio_copy.c

substdo.o: \
compile substdo.c substdio.h str.h byte.h error.h
	./compile substdo.c

syslog.lib: \
trysyslog.c compile load
	( ( ./compile trysyslog.c && \
	./load trysyslog -lgen ) >/dev/null 2>&1 \
	&& echo -lgen || exit 0 ) > syslog.lib
	rm -f trysyslog.o trysyslog

systype: \
find-systype trycpp.c
	./find-systype > systype

tcp-env: \
load tcp-env.o dns.o remoteinfo.o timeoutread.o timeoutwrite.o \
timeoutconn.o ip.o ipalloc.o case.a ndelay.a sig.a env.a getopt.a \
stralloc.a alloc.a substdio.a error.a str.a fs.a dns.lib socket.lib
	./load tcp-env dns.o remoteinfo.o timeoutread.o \
	timeoutwrite.o timeoutconn.o ip.o ipalloc.o case.a ndelay.a \
	sig.a env.a getopt.a stralloc.a alloc.a substdio.a error.a \
	str.a fs.a  `cat dns.lib` `cat socket.lib`

tcp-env.0: \
tcp-env.1
	nroff -man tcp-env.1 > tcp-env.0

tcp-env.o: \
compile tcp-env.c sig.h stralloc.h gen_alloc.h str.h env.h fmt.h \
scan.h subgetopt.h ip.h dns.h byte.h remoteinfo.h exit.h case.h
	./compile tcp-env.c

tcp-environ.0: \
tcp-environ.5
	nroff -man tcp-environ.5 > tcp-environ.0

tcpto.o: \
compile tcpto.c tcpto.h open.h lock.h seek.h now.h datetime.h ip.h \
byte.h datetime.h readwrite.h
	./compile tcpto.c

tcpto_clean.o: \
compile tcpto_clean.c tcpto.h open.h substdio.h readwrite.h
	./compile tcpto_clean.c

timeoutconn.o: \
compile timeoutconn.c ndelay.h select.h error.h readwrite.h ip.h \
byte.h timeoutconn.h
	./compile timeoutconn.c

timeoutread.o: \
compile timeoutread.c timeoutread.h select.h error.h readwrite.h
	./compile timeoutread.c

timeoutwrite.o: \
compile timeoutwrite.c timeoutwrite.h select.h error.h readwrite.h
	./compile timeoutwrite.c

token822.o: \
compile token822.c stralloc.h gen_alloc.h alloc.h str.h token822.h \
gen_alloc.h gen_allocdefs.h
	./compile token822.c

trigger.o: \
compile trigger.c select.h open.h trigger.h hasnpbg1.h
	./compile trigger.c

triggerpull.o: \
compile triggerpull.c ndelay.h open.h triggerpull.h
	./compile triggerpull.c

uint32.h: \
tryulong32.c compile load uint32.h1 uint32.h2
	( ( ./compile tryulong32.c && ./load tryulong32 && \
	./tryulong32 ) >/dev/null 2>&1 \
	&& cat uint32.h2 || cat uint32.h1 ) > uint32.h
	rm -f tryulong32.o tryulong32

wait.a: \
makelib wait_pid.o wait_nohang.o
	./makelib wait.a wait_pid.o wait_nohang.o

wait_nohang.o: \
compile wait_nohang.c haswaitp.h
	./compile wait_nohang.c

wait_pid.o: \
compile wait_pid.c error.h haswaitp.h
	./compile wait_pid.c
