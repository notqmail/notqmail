SHELL=/bin/sh

default: it

addresses.0: \
addresses.5
	nroff -man addresses.5 > addresses.0

alloc.a: \
makelib alloc.o alloc_re.o
	./makelib alloc.a alloc.o alloc_re.o

alloc.o: \
compile alloc.c alloc.h alloc.c error.h alloc.c
	./compile alloc.c

alloc_re.o: \
compile alloc_re.c alloc.h alloc_re.c byte.h alloc_re.c
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
compile auto-gid.c auto-gid.c auto-gid.c subfd.h substdio.h subfd.h \
auto-gid.c substdio.h substdio.h auto-gid.c readwrite.h auto-gid.c \
exit.h auto-gid.c scan.h auto-gid.c fmt.h auto-gid.c
	./compile auto-gid.c

auto-int: \
load auto-int.o substdio.a error.a str.a fs.a
	./load auto-int substdio.a error.a str.a fs.a 

auto-int.o: \
compile auto-int.c substdio.h auto-int.c readwrite.h auto-int.c \
exit.h auto-int.c scan.h auto-int.c fmt.h auto-int.c
	./compile auto-int.c

auto-int8: \
load auto-int8.o substdio.a error.a str.a fs.a
	./load auto-int8 substdio.a error.a str.a fs.a 

auto-int8.o: \
compile auto-int8.c substdio.h auto-int8.c readwrite.h auto-int8.c \
exit.h auto-int8.c scan.h auto-int8.c fmt.h auto-int8.c
	./compile auto-int8.c

auto-str: \
load auto-str.o substdio.a error.a str.a
	./load auto-str substdio.a error.a str.a 

auto-str.o: \
compile auto-str.c substdio.h auto-str.c readwrite.h auto-str.c \
exit.h auto-str.c
	./compile auto-str.c

auto-uid: \
load auto-uid.o substdio.a error.a str.a fs.a
	./load auto-uid substdio.a error.a str.a fs.a 

auto-uid.o: \
compile auto-uid.c auto-uid.c auto-uid.c subfd.h substdio.h subfd.h \
auto-uid.c substdio.h substdio.h auto-uid.c readwrite.h auto-uid.c \
exit.h auto-uid.c scan.h auto-uid.c fmt.h auto-uid.c
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
	) > auto_uids.c

auto_uids.o: \
compile auto_uids.c
	./compile auto_uids.c

auto_usera.c: \
auto-str conf-users
	./auto-str auto_usera `head -1 conf-users` > auto_usera.c

auto_usera.o: \
compile auto_usera.c
	./compile auto_usera.c

byte_chr.o: \
compile byte_chr.c byte.h byte_chr.c
	./compile byte_chr.c

byte_copy.o: \
compile byte_copy.c byte.h byte_copy.c
	./compile byte_copy.c

byte_cr.o: \
compile byte_cr.c byte.h byte_cr.c
	./compile byte_cr.c

byte_diff.o: \
compile byte_diff.c byte.h byte_diff.c
	./compile byte_diff.c

byte_rchr.o: \
compile byte_rchr.c byte.h byte_rchr.c
	./compile byte_rchr.c

byte_zero.o: \
compile byte_zero.c byte.h byte_zero.c
	./compile byte_zero.c

case.a: \
makelib case_diffb.o case_diffs.o case_lowerb.o case_lowers.o \
case_starts.o
	./makelib case.a case_diffb.o case_diffs.o case_lowerb.o \
	case_lowers.o case_starts.o

case_diffb.o: \
compile case_diffb.c case.h case_diffb.c
	./compile case_diffb.c

case_diffs.o: \
compile case_diffs.c case.h case_diffs.c
	./compile case_diffs.c

case_lowerb.o: \
compile case_lowerb.c case.h case_lowerb.c
	./compile case_lowerb.c

case_lowers.o: \
compile case_lowers.c case.h case_lowers.c
	./compile case_lowers.c

case_starts.o: \
compile case_starts.c case.h case_starts.c
	./compile case_starts.c

cdb.a: \
makelib cdb_hash.o cdb_unpack.o cdb_seek.o
	./makelib cdb.a cdb_hash.o cdb_unpack.o cdb_seek.o

cdb_hash.o: \
compile cdb_hash.c cdb.h uint32.h cdb.h cdb_hash.c
	./compile cdb_hash.c

cdb_seek.o: \
compile cdb_seek.c cdb_seek.c cdb_seek.c cdb.h uint32.h cdb.h \
cdb_seek.c
	./compile cdb_seek.c

cdb_unpack.o: \
compile cdb_unpack.c cdb.h uint32.h cdb.h cdb_unpack.c
	./compile cdb_unpack.c

cdbmake.a: \
makelib cdbmake_pack.o cdbmake_hash.o cdbmake_add.o
	./makelib cdbmake.a cdbmake_pack.o cdbmake_hash.o \
	cdbmake_add.o

cdbmake_add.o: \
compile cdbmake_add.c cdbmake.h uint32.h cdbmake.h cdbmake_add.c
	./compile cdbmake_add.c

cdbmake_hash.o: \
compile cdbmake_hash.c cdbmake.h uint32.h cdbmake.h cdbmake_hash.c
	./compile cdbmake_hash.c

cdbmake_pack.o: \
compile cdbmake_pack.c cdbmake.h uint32.h cdbmake.h cdbmake_pack.c
	./compile cdbmake_pack.c

cdbmss.o: \
compile cdbmss.c readwrite.h cdbmss.c seek.h cdbmss.c alloc.h \
cdbmss.c cdbmss.h cdbmake.h uint32.h cdbmake.h cdbmss.h substdio.h \
cdbmss.h cdbmss.c
	./compile cdbmss.c

check: \
it man conf-qmail
	./qmail-hier | ./instcheck `head -1 conf-qmail`

chkshsgr: \
load chkshsgr.o
	./load chkshsgr 

chkshsgr.o: \
compile chkshsgr.c exit.h chkshsgr.c
	./compile chkshsgr.c

chkspawn: \
load chkspawn.o substdio.a error.a str.a fs.a auto_spawn.o
	./load chkspawn substdio.a error.a str.a fs.a auto_spawn.o 

chkspawn.o: \
compile chkspawn.c substdio.h chkspawn.c subfd.h substdio.h \
substdio.h subfd.h chkspawn.c fmt.h chkspawn.c select.h select.h \
select.h select.h chkspawn.c exit.h chkspawn.c auto_spawn.h \
chkspawn.c
	./compile chkspawn.c

clean: \
TARGETS
	rm -f `cat TARGETS`

coe.o: \
compile coe.c coe.c coe.h coe.c
	./compile coe.c

compile: \
make-compile warn-auto.sh systype
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

condredirect: \
load condredirect.o qmail.o fd.a sig.a wait.a seek.a env.a alloc.a \
substdio.a error.a str.a auto_qmail.o
	./load condredirect qmail.o fd.a sig.a wait.a seek.a env.a \
	alloc.a substdio.a error.a str.a auto_qmail.o 

condredirect.0: \
condredirect.1
	nroff -man condredirect.1 > condredirect.0

condredirect.o: \
compile condredirect.c sig.h condredirect.c readwrite.h \
condredirect.c exit.h condredirect.c env.h condredirect.c error.h \
condredirect.c fork.h condredirect.c wait.h condredirect.c seek.h \
condredirect.c qmail.h substdio.h qmail.h condredirect.c stralloc.h \
gen_alloc.h stralloc.h condredirect.c subfd.h substdio.h substdio.h \
subfd.h condredirect.c substdio.h substdio.h condredirect.c
	./compile condredirect.c

constmap.o: \
compile constmap.c constmap.h constmap.c alloc.h constmap.c case.h \
constmap.c
	./compile constmap.c

control.o: \
compile control.c readwrite.h control.c open.h control.c getln.h \
control.c stralloc.h gen_alloc.h stralloc.h control.c substdio.h \
control.c error.h control.c control.h control.c alloc.h control.c \
scan.h control.c
	./compile control.c

date822fmt.o: \
compile date822fmt.c datetime.h date822fmt.c fmt.h date822fmt.c \
date822fmt.h date822fmt.c
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
compile datetime.c datetime.h datetime.c
	./compile datetime.c

datetime_un.o: \
compile datetime_un.c datetime.h datetime_un.c
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
compile dns.c dns.c dns.c dns.c dns.c dns.c dns.c dns.c ip.h dns.c \
ipalloc.h ip.h ip.h ipalloc.h gen_alloc.h ipalloc.h dns.c fmt.h dns.c \
alloc.h dns.c str.h dns.c stralloc.h gen_alloc.h stralloc.h dns.c \
dns.h dns.c case.h dns.c
	./compile dns.c

dnscname: \
load dnscname.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnscname dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnscname.o: \
compile dnscname.c substdio.h dnscname.c subfd.h substdio.h \
substdio.h subfd.h dnscname.c stralloc.h gen_alloc.h stralloc.h \
dnscname.c dns.h dnscname.c dnsdoe.h dnscname.c readwrite.h \
dnscname.c exit.h dnscname.c
	./compile dnscname.c

dnsdoe.o: \
compile dnsdoe.c substdio.h dnsdoe.c subfd.h substdio.h substdio.h \
subfd.h dnsdoe.c exit.h dnsdoe.c dns.h dnsdoe.c dnsdoe.h dnsdoe.c
	./compile dnsdoe.c

dnsfq: \
load dnsfq.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsfq dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnsfq.o: \
compile dnsfq.c substdio.h dnsfq.c subfd.h substdio.h substdio.h \
subfd.h dnsfq.c stralloc.h gen_alloc.h stralloc.h dnsfq.c dns.h \
dnsfq.c dnsdoe.h dnsfq.c ip.h dnsfq.c ipalloc.h ip.h ip.h ipalloc.h \
gen_alloc.h ipalloc.h dnsfq.c exit.h dnsfq.c
	./compile dnsfq.c

dnsip: \
load dnsip.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsip dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnsip.o: \
compile dnsip.c substdio.h dnsip.c subfd.h substdio.h substdio.h \
subfd.h dnsip.c stralloc.h gen_alloc.h stralloc.h dnsip.c dns.h \
dnsip.c dnsdoe.h dnsip.c ip.h dnsip.c ipalloc.h ip.h ip.h ipalloc.h \
gen_alloc.h ipalloc.h dnsip.c exit.h dnsip.c
	./compile dnsip.c

dnsmxip: \
load dnsmxip.o dns.o dnsdoe.o ip.o ipalloc.o now.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsmxip dns.o dnsdoe.o ip.o ipalloc.o now.o \
	stralloc.a alloc.a substdio.a error.a str.a fs.a  `cat \
	dns.lib` `cat socket.lib`

dnsmxip.o: \
compile dnsmxip.c substdio.h dnsmxip.c subfd.h substdio.h substdio.h \
subfd.h dnsmxip.c stralloc.h gen_alloc.h stralloc.h dnsmxip.c fmt.h \
dnsmxip.c dns.h dnsmxip.c dnsdoe.h dnsmxip.c ip.h dnsmxip.c ipalloc.h \
ip.h ip.h ipalloc.h gen_alloc.h ipalloc.h dnsmxip.c now.h datetime.h \
now.h dnsmxip.c exit.h dnsmxip.c
	./compile dnsmxip.c

dnsptr: \
load dnsptr.o dns.o dnsdoe.o ip.o ipalloc.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsptr dns.o dnsdoe.o ip.o ipalloc.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib` `cat \
	socket.lib`

dnsptr.o: \
compile dnsptr.c substdio.h dnsptr.c subfd.h substdio.h substdio.h \
subfd.h dnsptr.c stralloc.h gen_alloc.h stralloc.h dnsptr.c str.h \
dnsptr.c scan.h dnsptr.c dns.h dnsptr.c dnsdoe.h dnsptr.c ip.h \
dnsptr.c exit.h dnsptr.c
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
compile env.c str.h env.c alloc.h env.c env.h env.c
	./compile env.c

envelopes.0: \
envelopes.5
	nroff -man envelopes.5 > envelopes.0

envread.o: \
compile envread.c env.h envread.c str.h envread.c
	./compile envread.c

error.a: \
makelib error.o error_str.o error_temp.o
	./makelib error.a error.o error_str.o error_temp.o

error.o: \
compile error.c error.c error.h error.c
	./compile error.c

error_str.o: \
compile error_str.c error_str.c error.h error_str.c
	./compile error_str.c

error_temp.o: \
compile error_temp.c error_temp.c error.h error_temp.c
	./compile error_temp.c

fd.a: \
makelib fd_copy.o fd_move.o
	./makelib fd.a fd_copy.o fd_move.o

fd_copy.o: \
compile fd_copy.c fd_copy.c fd.h fd_copy.c
	./compile fd_copy.c

fd_move.o: \
compile fd_move.c fd.h fd_move.c
	./compile fd_move.c

fifo.o: \
compile fifo.c fifo.c fifo.c hasmkffo.h fifo.c fifo.h fifo.c
	./compile fifo.c

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

fmt_str.o: \
compile fmt_str.c fmt.h fmt_str.c
	./compile fmt_str.c

fmt_strn.o: \
compile fmt_strn.c fmt.h fmt_strn.c
	./compile fmt_strn.c

fmt_uint.o: \
compile fmt_uint.c fmt.h fmt_uint.c
	./compile fmt_uint.c

fmt_uint0.o: \
compile fmt_uint0.c fmt.h fmt_uint0.c
	./compile fmt_uint0.c

fmt_ulong.o: \
compile fmt_ulong.c fmt.h fmt_ulong.c
	./compile fmt_ulong.c

fmtqfn.o: \
compile fmtqfn.c fmtqfn.h fmtqfn.c fmt.h fmtqfn.c auto_split.h \
fmtqfn.c
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
load forward.o stralloc.a alloc.a qmail.o fd.a wait.a sig.a env.a \
substdio.a error.a str.a auto_qmail.o
	./load forward stralloc.a alloc.a qmail.o fd.a wait.a \
	sig.a env.a substdio.a error.a str.a auto_qmail.o 

forward.0: \
forward.1
	nroff -man forward.1 > forward.0

forward.o: \
compile forward.c sig.h forward.c readwrite.h forward.c exit.h \
forward.c env.h forward.c qmail.h substdio.h qmail.h forward.c \
stralloc.h gen_alloc.h stralloc.h forward.c subfd.h substdio.h \
substdio.h subfd.h forward.c substdio.h substdio.h forward.c
	./compile forward.c

fs.a: \
makelib fmt_str.o fmt_strn.o fmt_uint.o fmt_uint0.o fmt_ulong.o \
scan_ulong.o scan_8long.o scan_nbblong.o
	./makelib fs.a fmt_str.o fmt_strn.o fmt_uint.o fmt_uint0.o \
	fmt_ulong.o scan_ulong.o scan_8long.o scan_nbblong.o

getln.a: \
makelib getln.o getln2.o
	./makelib getln.a getln.o getln2.o

getln.o: \
compile getln.c substdio.h getln.c byte.h getln.c stralloc.h \
gen_alloc.h stralloc.h getln.c getln.h getln.c
	./compile getln.c

getln2.o: \
compile getln2.c substdio.h getln2.c stralloc.h gen_alloc.h \
stralloc.h getln2.c byte.h getln2.c getln.h getln2.c
	./compile getln2.c

getopt.a: \
makelib subgetopt.o sgetopt.o
	./makelib getopt.a subgetopt.o sgetopt.o

gfrom.o: \
compile gfrom.c str.h gfrom.c gfrom.h gfrom.c
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
compile headerbody.c stralloc.h gen_alloc.h stralloc.h headerbody.c \
substdio.h headerbody.c getln.h headerbody.c hfield.h headerbody.c \
headerbody.h headerbody.c
	./compile headerbody.c

hfield.o: \
compile hfield.c hfield.h hfield.c
	./compile hfield.c

hostname: \
load hostname.o substdio.a error.a str.a dns.lib socket.lib
	./load hostname substdio.a error.a str.a  `cat dns.lib` \
	`cat socket.lib`

hostname.o: \
compile hostname.c substdio.h hostname.c subfd.h substdio.h \
substdio.h subfd.h hostname.c readwrite.h hostname.c exit.h \
hostname.c
	./compile hostname.c

install: \
load install.o fifo.o getln.a strerr.a substdio.a stralloc.a alloc.a \
open.a error.a str.a fs.a
	./load install fifo.o getln.a strerr.a substdio.a \
	stralloc.a alloc.a open.a error.a str.a fs.a 

install.o: \
compile install.c substdio.h install.c stralloc.h gen_alloc.h \
stralloc.h install.c getln.h install.c readwrite.h install.c exit.h \
install.c open.h install.c error.h install.c strerr.h install.c \
byte.h install.c fifo.h install.c
	./compile install.c

instcheck: \
load instcheck.o getln.a strerr.a substdio.a stralloc.a alloc.a \
error.a str.a fs.a
	./load instcheck getln.a strerr.a substdio.a stralloc.a \
	alloc.a error.a str.a fs.a 

instcheck.o: \
compile instcheck.c instcheck.c instcheck.c substdio.h instcheck.c \
stralloc.h gen_alloc.h stralloc.h instcheck.c getln.h instcheck.c \
readwrite.h instcheck.c exit.h instcheck.c error.h instcheck.c \
strerr.h instcheck.c byte.h instcheck.c
	./compile instcheck.c

ip.o: \
compile ip.c fmt.h ip.c scan.h ip.c ip.h ip.c
	./compile ip.c

ipalloc.o: \
compile ipalloc.c alloc.h ipalloc.c gen_allocdefs.h gen_allocdefs.h \
gen_allocdefs.h ipalloc.c ip.h ipalloc.c ipalloc.h ip.h ip.h \
ipalloc.h gen_alloc.h ipalloc.h ipalloc.c
	./compile ipalloc.c

ipme.o: \
compile ipme.c ipme.c ipme.c ipme.c ipme.c ipme.c ipme.c ipme.c \
hassalen.h ipme.c byte.h ipme.c ip.h ipme.c ipalloc.h ip.h ip.h \
ipalloc.h gen_alloc.h ipalloc.h ipme.c stralloc.h gen_alloc.h \
stralloc.h ipme.c ipme.h ip.h ip.h ipme.h ipalloc.h ipalloc.h ipme.h \
ipme.c ipme.c
	./compile ipme.c

ipmeprint: \
load ipmeprint.o ipme.o ip.o ipalloc.o stralloc.a alloc.a substdio.a \
error.a str.a fs.a socket.lib
	./load ipmeprint ipme.o ip.o ipalloc.o stralloc.a alloc.a \
	substdio.a error.a str.a fs.a  `cat socket.lib`

ipmeprint.o: \
compile ipmeprint.c subfd.h substdio.h subfd.h ipmeprint.c substdio.h \
substdio.h ipmeprint.c ip.h ipmeprint.c ipme.h ip.h ip.h ipme.h \
ipalloc.h ip.h ip.h ipalloc.h gen_alloc.h ipalloc.h ipme.h \
ipmeprint.c exit.h ipmeprint.c
	./compile ipmeprint.c

it: \
qmail-local qmail-lspawn qmail-getpw qmail-remote qmail-rspawn \
qmail-clean qmail-send qmail-start splogger qmail-queue qmail-inject \
predate datemail mailsubj qmail-upq qmail-config qmail-showctl \
qmail-newu qmail-pw2u qmail-qread qmail-qstat qmail-tcpto qmail-pop3d \
qmail-popup qmail-qmtpd qmail-smtpd sendmail tcp-env dnscname dnsptr \
dnsip dnsmxip dnsfq hostname ipmeprint qlist qlist2 qreceipt qsmhook \
qbiff forward preline condredirect maildirmake maildir2mbox \
maildirwatch qail elq pinq qmail-hier install instcheck

load: \
make-load warn-auto.sh systype
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

lock.a: \
makelib lock_ex.o lock_exnb.o lock_un.o
	./makelib lock.a lock_ex.o lock_exnb.o lock_un.o

lock_ex.o: \
compile lock_ex.c lock_ex.c lock_ex.c lock_ex.c hasflock.h lock_ex.c \
lock.h lock_ex.c
	./compile lock_ex.c

lock_exnb.o: \
compile lock_exnb.c lock_exnb.c lock_exnb.c lock_exnb.c hasflock.h \
lock_exnb.c lock.h lock_exnb.c
	./compile lock_exnb.c

lock_un.o: \
compile lock_un.c lock_un.c lock_un.c lock_un.c hasflock.h lock_un.c \
lock.h lock_un.c
	./compile lock_un.c

maildir.0: \
maildir.5
	nroff -man maildir.5 > maildir.0

maildir.o: \
compile maildir.c maildir.c maildir.c prioq.h datetime.h prioq.h \
gen_alloc.h prioq.h maildir.c env.h maildir.c stralloc.h gen_alloc.h \
stralloc.h maildir.c direntry.h direntry.h direntry.h maildir.c \
datetime.h datetime.h maildir.c now.h datetime.h datetime.h now.h \
maildir.c str.h maildir.c maildir.h strerr.h maildir.h maildir.c
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
compile maildir2mbox.c readwrite.h maildir2mbox.c prioq.h datetime.h \
prioq.h gen_alloc.h prioq.h maildir2mbox.c env.h maildir2mbox.c \
stralloc.h gen_alloc.h stralloc.h maildir2mbox.c subfd.h substdio.h \
subfd.h maildir2mbox.c substdio.h substdio.h maildir2mbox.c getln.h \
maildir2mbox.c error.h maildir2mbox.c open.h maildir2mbox.c lock.h \
maildir2mbox.c gfrom.h maildir2mbox.c str.h maildir2mbox.c exit.h \
maildir2mbox.c myctime.h maildir2mbox.c maildir.h strerr.h maildir.h \
maildir2mbox.c
	./compile maildir2mbox.c

maildirmake: \
load maildirmake.o substdio.a error.a str.a
	./load maildirmake substdio.a error.a str.a 

maildirmake.0: \
maildirmake.1
	nroff -man maildirmake.1 > maildirmake.0

maildirmake.o: \
compile maildirmake.c subfd.h substdio.h subfd.h maildirmake.c \
substdio.h substdio.h maildirmake.c error.h maildirmake.c exit.h \
maildirmake.c
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
compile maildirwatch.c getln.h maildirwatch.c substdio.h \
maildirwatch.c subfd.h substdio.h substdio.h subfd.h maildirwatch.c \
prioq.h datetime.h prioq.h gen_alloc.h prioq.h maildirwatch.c \
stralloc.h gen_alloc.h stralloc.h maildirwatch.c str.h maildirwatch.c \
exit.h maildirwatch.c hfield.h maildirwatch.c readwrite.h \
maildirwatch.c open.h maildirwatch.c headerbody.h maildirwatch.c \
maildir.h strerr.h maildir.h maildirwatch.c
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
qmail-pw2u.0 qmail-qread.0 qmail-qstat.0 qmail-tcpto.0 qmail-pop3d.0 \
qmail-popup.0 qmail-qmtpd.0 qmail-smtpd.0 tcp-env.0 qlist.0 \
qreceipt.0 qbiff.0 forward.0 preline.0 condredirect.0 maildirmake.0 \
maildir2mbox.0 maildirwatch.0 qmail.0 qmail-upgrade.0 qmail-limits.0 \
qmail-log.0 qmail-control.0 qmail-header.0 qmail-users.0 dot-qmail.0 \
qmail-command.0 tcp-environ.0 maildir.0 mbox.0 addresses.0 \
envelopes.0 forgeries.0

mbox.0: \
mbox.5
	nroff -man mbox.5 > mbox.0

myctime.o: \
compile myctime.c datetime.h myctime.c fmt.h myctime.c myctime.h \
myctime.c
	./compile myctime.c

ndelay.a: \
makelib ndelay.o ndelay_off.o
	./makelib ndelay.a ndelay.o ndelay_off.o

ndelay.o: \
compile ndelay.c ndelay.c ndelay.c ndelay.h ndelay.c
	./compile ndelay.c

ndelay_off.o: \
compile ndelay_off.c ndelay_off.c ndelay_off.c ndelay.h ndelay_off.c
	./compile ndelay_off.c

newfield.o: \
compile newfield.c fmt.h newfield.c datetime.h newfield.c stralloc.h \
gen_alloc.h stralloc.h newfield.c date822fmt.h newfield.c newfield.h \
stralloc.h stralloc.h newfield.h newfield.c
	./compile newfield.c

now.o: \
compile now.c now.c datetime.h now.c now.h datetime.h datetime.h \
now.h now.c
	./compile now.c

open.a: \
makelib open_append.o open_excl.o open_read.o open_trunc.o \
open_write.o
	./makelib open.a open_append.o open_excl.o open_read.o \
	open_trunc.o open_write.o

open_append.o: \
compile open_append.c open_append.c open_append.c open.h \
open_append.c
	./compile open_append.c

open_excl.o: \
compile open_excl.c open_excl.c open_excl.c open.h open_excl.c
	./compile open_excl.c

open_read.o: \
compile open_read.c open_read.c open_read.c open.h open_read.c
	./compile open_read.c

open_trunc.o: \
compile open_trunc.c open_trunc.c open_trunc.c open.h open_trunc.c
	./compile open_trunc.c

open_write.o: \
compile open_write.c open_write.c open_write.c open.h open_write.c
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
load predate.o datetime.a sig.a fd.a wait.a substdio.a error.a str.a \
fs.a
	./load predate datetime.a sig.a fd.a wait.a substdio.a \
	error.a str.a fs.a 

predate.o: \
compile predate.c predate.c predate.c datetime.h predate.c fork.h \
predate.c wait.h predate.c fd.h predate.c fmt.h predate.c substdio.h \
predate.c subfd.h substdio.h substdio.h subfd.h predate.c readwrite.h \
predate.c exit.h predate.c
	./compile predate.c

preline: \
load preline.o fd.a wait.a sig.a env.a getopt.a substdio.a error.a \
str.a
	./load preline fd.a wait.a sig.a env.a getopt.a substdio.a \
	error.a str.a 

preline.0: \
preline.1
	nroff -man preline.1 > preline.0

preline.o: \
compile preline.c fd.h preline.c sgetopt.h subgetopt.h sgetopt.h \
preline.c readwrite.h preline.c subfd.h substdio.h subfd.h preline.c \
substdio.h substdio.h preline.c exit.h preline.c fork.h preline.c \
wait.h preline.c env.h preline.c sig.h preline.c error.h preline.c
	./compile preline.c

prioq.o: \
compile prioq.c alloc.h prioq.c gen_allocdefs.h gen_allocdefs.h \
gen_allocdefs.h prioq.c prioq.h datetime.h prioq.h gen_alloc.h \
prioq.h prioq.c
	./compile prioq.c

prot.o: \
compile prot.c hasshsgr.h prot.c prot.h prot.c
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
compile qbiff.c qbiff.c qbiff.c qbiff.c readwrite.h qbiff.c \
stralloc.h gen_alloc.h stralloc.h qbiff.c substdio.h qbiff.c subfd.h \
substdio.h substdio.h subfd.h qbiff.c open.h qbiff.c byte.h qbiff.c \
str.h qbiff.c headerbody.h qbiff.c hfield.h qbiff.c env.h qbiff.c \
exit.h qbiff.c
	./compile qbiff.c

qlist: \
load qlist.o headerbody.o hfield.o token822.o qmail.o getln.a env.a \
case.a sig.a fd.a wait.a open.a lock.a stralloc.a alloc.a substdio.a \
error.a str.a auto_qmail.o
	./load qlist headerbody.o hfield.o token822.o qmail.o \
	getln.a env.a case.a sig.a fd.a wait.a open.a lock.a \
	stralloc.a alloc.a substdio.a error.a str.a auto_qmail.o 

qlist.0: \
qlist.1
	nroff -man qlist.1 > qlist.0

qlist.o: \
compile qlist.c sig.h qlist.c readwrite.h qlist.c substdio.h qlist.c \
stralloc.h gen_alloc.h stralloc.h qlist.c subfd.h substdio.h \
substdio.h subfd.h qlist.c getln.h qlist.c alloc.h qlist.c str.h \
qlist.c env.h qlist.c hfield.h qlist.c case.h qlist.c token822.h \
gen_alloc.h token822.h qlist.c error.h qlist.c gen_alloc.h qlist.c \
gen_allocdefs.h gen_allocdefs.h gen_allocdefs.h qlist.c headerbody.h \
qlist.c exit.h qlist.c open.h qlist.c lock.h qlist.c qmail.h \
substdio.h substdio.h qmail.h qlist.c qlist.c
	./compile qlist.c

qlist2: \
warn-auto.sh qlist2.sh conf-qmail conf-break conf-split
	cat warn-auto.sh qlist2.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qlist2
	chmod 755 qlist2

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
compile qmail-clean.c qmail-clean.c qmail-clean.c readwrite.h \
qmail-clean.c sig.h qmail-clean.c now.h datetime.h now.h \
qmail-clean.c str.h qmail-clean.c direntry.h direntry.h direntry.h \
qmail-clean.c getln.h qmail-clean.c stralloc.h gen_alloc.h stralloc.h \
qmail-clean.c substdio.h qmail-clean.c subfd.h substdio.h substdio.h \
subfd.h qmail-clean.c byte.h qmail-clean.c scan.h qmail-clean.c fmt.h \
qmail-clean.c error.h qmail-clean.c exit.h qmail-clean.c fmtqfn.h \
qmail-clean.c auto_qmail.h qmail-clean.c
	./compile qmail-clean.c

qmail-command.0: \
qmail-command.8
	nroff -man qmail-command.8 > qmail-command.0

qmail-config: \
warn-auto.sh qmail-config.sh conf-qmail conf-break conf-split
	cat warn-auto.sh qmail-config.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qmail-config
	chmod 755 qmail-config

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
compile qmail-getpw.c qmail-getpw.c qmail-getpw.c qmail-getpw.c \
readwrite.h qmail-getpw.c substdio.h qmail-getpw.c subfd.h substdio.h \
substdio.h subfd.h qmail-getpw.c error.h qmail-getpw.c exit.h \
qmail-getpw.c byte.h qmail-getpw.c str.h qmail-getpw.c case.h \
qmail-getpw.c fmt.h qmail-getpw.c auto_usera.h qmail-getpw.c \
auto_break.h qmail-getpw.c qlx.h qmail-getpw.c
	./compile qmail-getpw.c

qmail-header.0: \
qmail-header.5
	nroff -man qmail-header.5 > qmail-header.0

qmail-hier: \
load qmail-hier.o substdio.a error.a str.a fs.a auto_split.o \
auto_uids.o
	./load qmail-hier substdio.a error.a str.a fs.a \
	auto_split.o auto_uids.o 

qmail-hier.o: \
compile qmail-hier.c subfd.h substdio.h subfd.h qmail-hier.c \
substdio.h substdio.h qmail-hier.c auto_split.h qmail-hier.c \
auto_uids.h qmail-hier.c fmt.h qmail-hier.c
	./compile qmail-hier.c

qmail-inject: \
load qmail-inject.o headerbody.o hfield.o newfield.o quote.o now.o \
control.o date822fmt.o qmail.o fd.a wait.a open.a getln.a sig.a \
getopt.a datetime.a token822.o env.a stralloc.a alloc.a substdio.a \
error.a str.a fs.a auto_qmail.o
	./load qmail-inject headerbody.o hfield.o newfield.o \
	quote.o now.o control.o date822fmt.o qmail.o fd.a wait.a \
	open.a getln.a sig.a getopt.a datetime.a token822.o env.a \
	stralloc.a alloc.a substdio.a error.a str.a fs.a \
	auto_qmail.o 

qmail-inject.0: \
qmail-inject.8
	nroff -man qmail-inject.8 > qmail-inject.0

qmail-inject.o: \
compile qmail-inject.c sig.h qmail-inject.c substdio.h qmail-inject.c \
stralloc.h gen_alloc.h stralloc.h qmail-inject.c subfd.h substdio.h \
substdio.h subfd.h qmail-inject.c sgetopt.h subgetopt.h sgetopt.h \
qmail-inject.c getln.h qmail-inject.c alloc.h qmail-inject.c str.h \
qmail-inject.c fmt.h qmail-inject.c hfield.h qmail-inject.c \
token822.h gen_alloc.h token822.h qmail-inject.c control.h \
qmail-inject.c env.h qmail-inject.c gen_alloc.h qmail-inject.c \
gen_allocdefs.h gen_allocdefs.h gen_allocdefs.h qmail-inject.c \
error.h qmail-inject.c qmail.h substdio.h substdio.h qmail.h \
qmail-inject.c now.h datetime.h now.h qmail-inject.c exit.h \
qmail-inject.c quote.h qmail-inject.c headerbody.h qmail-inject.c \
auto_qmail.h qmail-inject.c newfield.h stralloc.h stralloc.h \
newfield.h qmail-inject.c
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
wait.a env.a stralloc.a alloc.a substdio.a error.a str.a fs.a \
datetime.a auto_qmail.o auto_patrn.o socket.lib
	./load qmail-local qmail.o quote.o now.o gfrom.o myctime.o \
	slurpclose.o case.a getln.a getopt.a sig.a open.a seek.a \
	lock.a fd.a wait.a env.a stralloc.a alloc.a substdio.a \
	error.a str.a fs.a datetime.a auto_qmail.o auto_patrn.o  \
	`cat socket.lib`

qmail-local.0: \
qmail-local.8
	nroff -man qmail-local.8 > qmail-local.0

qmail-local.o: \
compile qmail-local.c qmail-local.c qmail-local.c readwrite.h \
qmail-local.c sig.h qmail-local.c env.h qmail-local.c byte.h \
qmail-local.c exit.h qmail-local.c fork.h qmail-local.c open.h \
qmail-local.c wait.h qmail-local.c lock.h qmail-local.c seek.h \
qmail-local.c substdio.h qmail-local.c getln.h qmail-local.c subfd.h \
substdio.h substdio.h subfd.h qmail-local.c sgetopt.h subgetopt.h \
sgetopt.h qmail-local.c alloc.h qmail-local.c error.h qmail-local.c \
stralloc.h gen_alloc.h stralloc.h qmail-local.c fmt.h qmail-local.c \
str.h qmail-local.c now.h datetime.h now.h qmail-local.c case.h \
qmail-local.c quote.h qmail-local.c qmail.h substdio.h substdio.h \
qmail.h qmail-local.c slurpclose.h qmail-local.c myctime.h \
qmail-local.c gfrom.h qmail-local.c auto_patrn.h qmail-local.c
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
compile qmail-lspawn.c fd.h qmail-lspawn.c wait.h qmail-lspawn.c \
prot.h qmail-lspawn.c substdio.h qmail-lspawn.c stralloc.h \
gen_alloc.h stralloc.h qmail-lspawn.c scan.h qmail-lspawn.c exit.h \
qmail-lspawn.c fork.h qmail-lspawn.c error.h qmail-lspawn.c cdb.h \
uint32.h cdb.h qmail-lspawn.c case.h qmail-lspawn.c slurpclose.h \
qmail-lspawn.c auto_qmail.h qmail-lspawn.c auto_uids.h qmail-lspawn.c \
qlx.h qmail-lspawn.c
	./compile qmail-lspawn.c

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
compile qmail-newu.c stralloc.h gen_alloc.h stralloc.h qmail-newu.c \
subfd.h substdio.h subfd.h qmail-newu.c getln.h qmail-newu.c \
substdio.h substdio.h qmail-newu.c cdbmss.h cdbmake.h uint32.h \
cdbmake.h cdbmss.h substdio.h substdio.h cdbmss.h qmail-newu.c exit.h \
qmail-newu.c readwrite.h qmail-newu.c open.h qmail-newu.c error.h \
qmail-newu.c case.h qmail-newu.c auto_qmail.h qmail-newu.c
	./compile qmail-newu.c

qmail-pop3d: \
load qmail-pop3d.o prioq.o now.o sig.a open.a getln.a stralloc.a \
alloc.a substdio.a error.a str.a fs.a
	./load qmail-pop3d prioq.o now.o sig.a open.a getln.a \
	stralloc.a alloc.a substdio.a error.a str.a fs.a 

qmail-pop3d.0: \
qmail-pop3d.8
	nroff -man qmail-pop3d.8 > qmail-pop3d.0

qmail-pop3d.o: \
compile qmail-pop3d.c qmail-pop3d.c qmail-pop3d.c direntry.h \
direntry.h direntry.h qmail-pop3d.c sig.h qmail-pop3d.c getln.h \
qmail-pop3d.c stralloc.h gen_alloc.h stralloc.h qmail-pop3d.c \
substdio.h qmail-pop3d.c alloc.h qmail-pop3d.c datetime.h \
qmail-pop3d.c prot.h qmail-pop3d.c open.h qmail-pop3d.c prioq.h \
datetime.h datetime.h prioq.h gen_alloc.h prioq.h qmail-pop3d.c \
scan.h qmail-pop3d.c fmt.h qmail-pop3d.c error.h qmail-pop3d.c str.h \
qmail-pop3d.c exit.h qmail-pop3d.c now.h datetime.h datetime.h now.h \
qmail-pop3d.c readwrite.h qmail-pop3d.c
	./compile qmail-pop3d.c

qmail-popup: \
load qmail-popup.o now.o fd.a sig.a wait.a getln.a stralloc.a alloc.a \
substdio.a error.a str.a fs.a
	./load qmail-popup now.o fd.a sig.a wait.a getln.a \
	stralloc.a alloc.a substdio.a error.a str.a fs.a 

qmail-popup.0: \
qmail-popup.8
	nroff -man qmail-popup.8 > qmail-popup.0

qmail-popup.o: \
compile qmail-popup.c qmail-popup.c qmail-popup.c fd.h qmail-popup.c \
sig.h qmail-popup.c getln.h qmail-popup.c stralloc.h gen_alloc.h \
stralloc.h qmail-popup.c substdio.h qmail-popup.c subfd.h substdio.h \
substdio.h subfd.h qmail-popup.c alloc.h qmail-popup.c datetime.h \
qmail-popup.c error.h qmail-popup.c wait.h qmail-popup.c str.h \
qmail-popup.c now.h datetime.h datetime.h now.h qmail-popup.c fmt.h \
qmail-popup.c exit.h qmail-popup.c readwrite.h qmail-popup.c
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
compile qmail-pw2u.c qmail-pw2u.c qmail-pw2u.c substdio.h \
qmail-pw2u.c readwrite.h qmail-pw2u.c subfd.h substdio.h substdio.h \
subfd.h qmail-pw2u.c sgetopt.h subgetopt.h sgetopt.h qmail-pw2u.c \
control.h qmail-pw2u.c constmap.h qmail-pw2u.c stralloc.h gen_alloc.h \
stralloc.h qmail-pw2u.c fmt.h qmail-pw2u.c str.h qmail-pw2u.c scan.h \
qmail-pw2u.c open.h qmail-pw2u.c error.h qmail-pw2u.c getln.h \
qmail-pw2u.c auto_break.h qmail-pw2u.c auto_qmail.h qmail-pw2u.c \
auto_usera.h qmail-pw2u.c
	./compile qmail-pw2u.c

qmail-qmtpd: \
load qmail-qmtpd.o control.o constmap.o received.o date822fmt.o now.o \
qmail.o fd.a wait.a datetime.a open.a getln.a sig.a case.a env.a \
stralloc.a alloc.a substdio.a error.a str.a fs.a auto_qmail.o
	./load qmail-qmtpd control.o constmap.o received.o \
	date822fmt.o now.o qmail.o fd.a wait.a datetime.a open.a \
	getln.a sig.a case.a env.a stralloc.a alloc.a substdio.a \
	error.a str.a fs.a auto_qmail.o 

qmail-qmtpd.0: \
qmail-qmtpd.8
	nroff -man qmail-qmtpd.8 > qmail-qmtpd.0

qmail-qmtpd.o: \
compile qmail-qmtpd.c stralloc.h gen_alloc.h stralloc.h qmail-qmtpd.c \
substdio.h qmail-qmtpd.c subfd.h substdio.h substdio.h subfd.h \
qmail-qmtpd.c qmail.h substdio.h substdio.h qmail.h qmail-qmtpd.c \
now.h datetime.h now.h qmail-qmtpd.c str.h qmail-qmtpd.c fmt.h \
qmail-qmtpd.c env.h qmail-qmtpd.c sig.h qmail-qmtpd.c auto_qmail.h \
qmail-qmtpd.c now.h qmail-qmtpd.c datetime.h datetime.h qmail-qmtpd.c \
date822fmt.h qmail-qmtpd.c readwrite.h qmail-qmtpd.c control.h \
qmail-qmtpd.c constmap.h qmail-qmtpd.c received.h qmail-qmtpd.c
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
compile qmail-qread.c qmail-qread.c qmail-qread.c stralloc.h \
gen_alloc.h stralloc.h qmail-qread.c substdio.h qmail-qread.c subfd.h \
substdio.h substdio.h subfd.h qmail-qread.c fmt.h qmail-qread.c str.h \
qmail-qread.c getln.h qmail-qread.c fmtqfn.h qmail-qread.c \
readsubdir.h direntry.h direntry.h direntry.h readsubdir.h \
qmail-qread.c auto_qmail.h qmail-qread.c open.h qmail-qread.c \
datetime.h qmail-qread.c date822fmt.h qmail-qread.c readwrite.h \
qmail-qread.c error.h qmail-qread.c exit.h qmail-qread.c
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
compile qmail-queue.c qmail-queue.c qmail-queue.c readwrite.h \
qmail-queue.c sig.h qmail-queue.c exit.h qmail-queue.c open.h \
qmail-queue.c seek.h qmail-queue.c fmt.h qmail-queue.c alloc.h \
qmail-queue.c substdio.h qmail-queue.c datetime.h qmail-queue.c now.h \
datetime.h datetime.h now.h qmail-queue.c triggerpull.h qmail-queue.c \
extra.h qmail-queue.c auto_qmail.h qmail-queue.c auto_uids.h \
qmail-queue.c date822fmt.h qmail-queue.c fmtqfn.h qmail-queue.c
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
compile qmail-remote.c qmail-remote.c qmail-remote.c qmail-remote.c \
qmail-remote.c sig.h qmail-remote.c getln.h qmail-remote.c stralloc.h \
gen_alloc.h stralloc.h qmail-remote.c substdio.h qmail-remote.c \
subfd.h substdio.h substdio.h subfd.h qmail-remote.c scan.h \
qmail-remote.c case.h qmail-remote.c error.h qmail-remote.c \
auto_qmail.h qmail-remote.c control.h qmail-remote.c dns.h \
qmail-remote.c alloc.h qmail-remote.c quote.h qmail-remote.c ip.h \
qmail-remote.c ipalloc.h ip.h ip.h ipalloc.h gen_alloc.h ipalloc.h \
qmail-remote.c ipme.h ip.h ip.h ipme.h ipalloc.h ipalloc.h ipme.h \
qmail-remote.c gen_alloc.h qmail-remote.c gen_allocdefs.h \
gen_allocdefs.h gen_allocdefs.h qmail-remote.c str.h qmail-remote.c \
now.h datetime.h now.h qmail-remote.c exit.h qmail-remote.c \
constmap.h qmail-remote.c tcpto.h qmail-remote.c timeoutconn.h \
qmail-remote.c timeoutread.h qmail-remote.c timeoutwrite.h \
qmail-remote.c
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
compile qmail-rspawn.c fd.h qmail-rspawn.c wait.h qmail-rspawn.c \
substdio.h qmail-rspawn.c exit.h qmail-rspawn.c fork.h qmail-rspawn.c \
error.h qmail-rspawn.c tcpto.h qmail-rspawn.c
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
compile qmail-send.c qmail-send.c qmail-send.c readwrite.h \
qmail-send.c sig.h qmail-send.c direntry.h direntry.h direntry.h \
qmail-send.c control.h qmail-send.c select.h select.h select.h \
select.h qmail-send.c open.h qmail-send.c seek.h qmail-send.c exit.h \
qmail-send.c lock.h qmail-send.c ndelay.h qmail-send.c now.h \
datetime.h now.h qmail-send.c getln.h qmail-send.c substdio.h \
qmail-send.c alloc.h qmail-send.c error.h qmail-send.c stralloc.h \
gen_alloc.h stralloc.h qmail-send.c str.h qmail-send.c byte.h \
qmail-send.c fmt.h qmail-send.c scan.h qmail-send.c case.h \
qmail-send.c auto_qmail.h qmail-send.c trigger.h qmail-send.c \
newfield.h stralloc.h stralloc.h newfield.h qmail-send.c quote.h \
qmail-send.c qmail.h substdio.h substdio.h qmail.h qmail-send.c \
qsutil.h qmail-send.c prioq.h datetime.h datetime.h prioq.h \
gen_alloc.h prioq.h qmail-send.c constmap.h qmail-send.c fmtqfn.h \
qmail-send.c readsubdir.h direntry.h readsubdir.h qmail-send.c
	./compile qmail-send.c

qmail-showctl: \
load qmail-showctl.o control.o open.a getln.a stralloc.a alloc.a \
substdio.a error.a str.a fs.a auto_qmail.o
	./load qmail-showctl control.o open.a getln.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o 

qmail-showctl.0: \
qmail-showctl.8
	nroff -man qmail-showctl.8 > qmail-showctl.0

qmail-showctl.o: \
compile qmail-showctl.c substdio.h qmail-showctl.c subfd.h substdio.h \
substdio.h subfd.h qmail-showctl.c exit.h qmail-showctl.c fmt.h \
qmail-showctl.c str.h qmail-showctl.c control.h qmail-showctl.c \
constmap.h qmail-showctl.c stralloc.h gen_alloc.h stralloc.h \
qmail-showctl.c direntry.h direntry.h direntry.h qmail-showctl.c \
auto_qmail.h qmail-showctl.c
	./compile qmail-showctl.c

qmail-smtpd: \
load qmail-smtpd.o ip.o ipme.o ipalloc.o control.o constmap.o \
received.o date822fmt.o now.o qmail.o fd.a wait.a datetime.a open.a \
getln.a sig.a case.a env.a stralloc.a alloc.a substdio.a error.a \
str.a fs.a auto_qmail.o socket.lib
	./load qmail-smtpd ip.o ipme.o ipalloc.o control.o \
	constmap.o received.o date822fmt.o now.o qmail.o fd.a \
	wait.a datetime.a open.a getln.a sig.a case.a env.a \
	stralloc.a alloc.a substdio.a error.a str.a fs.a \
	auto_qmail.o  `cat socket.lib`

qmail-smtpd.0: \
qmail-smtpd.8
	nroff -man qmail-smtpd.8 > qmail-smtpd.0

qmail-smtpd.o: \
compile qmail-smtpd.c sig.h qmail-smtpd.c readwrite.h qmail-smtpd.c \
getln.h qmail-smtpd.c stralloc.h gen_alloc.h stralloc.h qmail-smtpd.c \
substdio.h qmail-smtpd.c alloc.h qmail-smtpd.c auto_qmail.h \
qmail-smtpd.c control.h qmail-smtpd.c received.h qmail-smtpd.c \
constmap.h qmail-smtpd.c error.h qmail-smtpd.c ipme.h ip.h ipme.h \
ipalloc.h ip.h ip.h ipalloc.h gen_alloc.h ipalloc.h ipme.h \
qmail-smtpd.c ip.h ip.h qmail-smtpd.c qmail.h substdio.h substdio.h \
qmail.h qmail-smtpd.c str.h qmail-smtpd.c fmt.h qmail-smtpd.c byte.h \
qmail-smtpd.c case.h qmail-smtpd.c env.h qmail-smtpd.c now.h \
datetime.h now.h qmail-smtpd.c exit.h qmail-smtpd.c
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
compile qmail-start.c fd.h qmail-start.c prot.h qmail-start.c exit.h \
qmail-start.c fork.h qmail-start.c auto_uids.h qmail-start.c
	./compile qmail-start.c

qmail-tcpto: \
load qmail-tcpto.o ip.o now.o open.a lock.a substdio.a error.a str.a \
fs.a auto_qmail.o
	./load qmail-tcpto ip.o now.o open.a lock.a substdio.a \
	error.a str.a fs.a auto_qmail.o 

qmail-tcpto.0: \
qmail-tcpto.8
	nroff -man qmail-tcpto.8 > qmail-tcpto.0

qmail-tcpto.o: \
compile qmail-tcpto.c substdio.h qmail-tcpto.c subfd.h substdio.h \
substdio.h subfd.h qmail-tcpto.c auto_qmail.h qmail-tcpto.c fmt.h \
qmail-tcpto.c ip.h qmail-tcpto.c lock.h qmail-tcpto.c error.h \
qmail-tcpto.c exit.h qmail-tcpto.c datetime.h qmail-tcpto.c now.h \
datetime.h datetime.h now.h qmail-tcpto.c
	./compile qmail-tcpto.c

qmail-upgrade.0: \
qmail-upgrade.7
	nroff -man qmail-upgrade.7 > qmail-upgrade.0

qmail-upgrade.7: \
qmail-upgrade.9 conf-break conf-spawn
	cat qmail-upgrade.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-upgrade.7

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
compile qmail.c substdio.h qmail.c readwrite.h qmail.c wait.h qmail.c \
exit.h qmail.c fork.h qmail.c fd.h qmail.c qmail.h substdio.h \
substdio.h qmail.h qmail.c auto_qmail.h qmail.c
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
compile qreceipt.c sig.h qreceipt.c env.h qreceipt.c substdio.h \
qreceipt.c stralloc.h gen_alloc.h stralloc.h qreceipt.c subfd.h \
substdio.h substdio.h subfd.h qreceipt.c getln.h qreceipt.c alloc.h \
qreceipt.c str.h qreceipt.c hfield.h qreceipt.c token822.h \
gen_alloc.h token822.h qreceipt.c error.h qreceipt.c gen_alloc.h \
qreceipt.c gen_allocdefs.h gen_allocdefs.h gen_allocdefs.h qreceipt.c \
headerbody.h qreceipt.c exit.h qreceipt.c open.h qreceipt.c quote.h \
qreceipt.c qmail.h substdio.h substdio.h qmail.h qreceipt.c
	./compile qreceipt.c

qsmhook: \
load qsmhook.o sig.a case.a fd.a wait.a getopt.a env.a stralloc.a \
alloc.a substdio.a error.a str.a
	./load qsmhook sig.a case.a fd.a wait.a getopt.a env.a \
	stralloc.a alloc.a substdio.a error.a str.a 

qsmhook.o: \
compile qsmhook.c fd.h qsmhook.c stralloc.h gen_alloc.h stralloc.h \
qsmhook.c readwrite.h qsmhook.c sgetopt.h subgetopt.h sgetopt.h \
qsmhook.c wait.h qsmhook.c env.h qsmhook.c byte.h qsmhook.c str.h \
qsmhook.c alloc.h qsmhook.c exit.h qsmhook.c fork.h qsmhook.c case.h \
qsmhook.c subfd.h substdio.h subfd.h qsmhook.c error.h qsmhook.c \
substdio.h substdio.h qsmhook.c sig.h qsmhook.c
	./compile qsmhook.c

qsutil.o: \
compile qsutil.c stralloc.h gen_alloc.h stralloc.h qsutil.c \
readwrite.h qsutil.c substdio.h qsutil.c qsutil.h qsutil.c
	./compile qsutil.c

quote.o: \
compile quote.c stralloc.h gen_alloc.h stralloc.h quote.c str.h \
quote.c quote.h quote.c
	./compile quote.c

readsubdir.o: \
compile readsubdir.c readsubdir.h direntry.h direntry.h direntry.h \
readsubdir.h readsubdir.c fmt.h readsubdir.c scan.h readsubdir.c \
str.h readsubdir.c auto_split.h readsubdir.c
	./compile readsubdir.c

received.o: \
compile received.c fmt.h received.c qmail.h substdio.h qmail.h \
received.c now.h datetime.h now.h received.c datetime.h datetime.h \
received.c date822fmt.h received.c received.h received.c
	./compile received.c

remoteinfo.o: \
compile remoteinfo.c remoteinfo.c remoteinfo.c remoteinfo.c \
remoteinfo.c byte.h remoteinfo.c substdio.h remoteinfo.c ip.h \
remoteinfo.c fmt.h remoteinfo.c timeoutconn.h remoteinfo.c \
timeoutread.h remoteinfo.c timeoutwrite.h remoteinfo.c remoteinfo.h \
remoteinfo.c
	./compile remoteinfo.c

scan_8long.o: \
compile scan_8long.c scan.h scan_8long.c
	./compile scan_8long.c

scan_nbblong.o: \
compile scan_nbblong.c scan.h scan_nbblong.c
	./compile scan_nbblong.c

scan_ulong.o: \
compile scan_ulong.c scan.h scan_ulong.c
	./compile scan_ulong.c

seek.a: \
makelib seek_cur.o seek_end.o seek_set.o seek_trunc.o
	./makelib seek.a seek_cur.o seek_end.o seek_set.o \
	seek_trunc.o

seek_cur.o: \
compile seek_cur.c seek_cur.c seek.h seek_cur.c
	./compile seek_cur.c

seek_end.o: \
compile seek_end.c seek_end.c seek.h seek_end.c
	./compile seek_end.c

seek_set.o: \
compile seek_set.c seek_set.c seek.h seek_set.c
	./compile seek_set.c

seek_trunc.o: \
compile seek_trunc.c seek_trunc.c seek.h seek_trunc.c
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
compile sendmail.c sgetopt.h subgetopt.h sgetopt.h sendmail.c \
substdio.h sendmail.c subfd.h substdio.h substdio.h subfd.h \
sendmail.c alloc.h sendmail.c auto_qmail.h sendmail.c exit.h \
sendmail.c env.h sendmail.c str.h sendmail.c
	./compile sendmail.c

setup: \
it man conf-qmail
	./qmail-hier | ./install `head -1 conf-qmail`

sgetopt.o: \
compile sgetopt.c substdio.h sgetopt.c subfd.h substdio.h substdio.h \
subfd.h sgetopt.c sgetopt.h sgetopt.h subgetopt.h sgetopt.h sgetopt.c \
subgetopt.h subgetopt.h sgetopt.c
	./compile sgetopt.c

shar: \
FILES BLURB BLURB2 BLURB3 BLURB4 README FAQ INSTALL INSTALL.alias \
INSTALL.boot INSTALL.ctl INSTALL.ids INSTALL.mbox INSTALL.qsmhook \
UPGRADE THOUGHTS TODO THANKS CHANGES RFCHCSC RFCLOOPS RFCMXPS \
RFCNETSTR RFCNRUDT RFCQMTP RFCQSBMF RFCVERP SECURITY INTERNALS FILES \
VERSION SYSDEPS TARGETS Makefile conf-break auto_break.h conf-spawn \
auto_spawn.h chkspawn.c conf-split auto_split.h conf-patrn \
auto_patrn.h conf-users conf-groups auto_uids.h auto_usera.h extra.h \
addresses.5 condredirect.1 dot-qmail.9 envelopes.5 forgeries.7 \
forward.1 maildir2mbox.1 maildirmake.1 maildirwatch.1 mailsubj.1 \
mbox.5 preline.1 qbiff.1 qlist.1 qmail-clean.8 qmail-command.8 \
qmail-control.5 qmail-getpw.9 qmail-header.5 qmail-inject.8 \
qmail-limits.9 qmail-local.8 qmail-log.5 qmail-lspawn.8 qmail-newu.8 \
qmail-pop3d.8 qmail-popup.8 qmail-pw2u.9 qmail-qmtpd.8 qmail-qread.8 \
qmail-qstat.8 qmail-queue.8 qmail-remote.8 qmail-rspawn.8 \
qmail-send.9 qmail-showctl.8 qmail-smtpd.8 qmail-start.8 \
qmail-tcpto.8 qmail-upgrade.9 qmail-users.5 qmail.7 qreceipt.1 \
splogger.8 tcp-env.1 qmail-clean.c qmail-config.sh qmail-getpw.c \
qmail-hier.c qmail-inject.c qmail-local.c qmail-lspawn.c qmail-newu.c \
qmail-pop3d.c qmail-popup.c qmail-pw2u.c qmail-qmtpd.c qmail-qread.c \
qmail-qstat.sh qmail-queue.c qmail-remote.c qmail-rspawn.c \
qmail-send.c qmail-showctl.c qmail-smtpd.c qmail-start.c \
qmail-tcpto.c spawn.c dnscname.c dnsfq.c dnsip.c dnsmxip.c dnsptr.c \
hostname.c ipmeprint.c tcp-env.c sendmail.c qlist.c qreceipt.c \
qsmhook.c qbiff.c forward.c preline.c predate.c condredirect.c \
maildirmake.c maildir2mbox.c maildirwatch.c splogger.c qail.sh elq.sh \
pinq.sh qlist2.sh qmail-upq.sh datemail.sh mailsubj.sh qlx.h \
constmap.h constmap.c dnsdoe.h dnsdoe.c fmtqfn.h fmtqfn.c gfrom.h \
gfrom.c myctime.h myctime.c newfield.h newfield.c qsutil.h qsutil.c \
readsubdir.h readsubdir.c received.h received.c tcpto.h tcpto.c \
tcpto_clean.c trigger.h trigger.c triggerpull.h triggerpull.c \
trynpbg1.c trysyslog.c conf-cc conf-ld find-systype.sh \
make-compile.sh make-load.sh make-makelib.sh trycpp.c warn-auto.sh \
auto-str.c auto-int.c auto-int8.c auto-gid.c auto-uid.c install.c \
instcheck.c alloc.3 alloc.h alloc.c alloc_re.c case.3 case.h \
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
scan_nbblong.c slurpclose.h slurpclose.c quote.h quote.c hfield.h \
hfield.c headerbody.h headerbody.c token822.h token822.c control.h \
control.c datetime.3 datetime.h datetime.c datetime_un.c prioq.h \
prioq.c date822fmt.h date822fmt.c dns.h dns.c trylsock.c tryrsolv.c \
ip.h ip.c ipalloc.h ipalloc.c select.h1 select.h2 trysysel.c ndelay.h \
ndelay.c ndelay_off.c direntry.3 direntry.h1 direntry.h2 trydrent.c \
prot.h prot.c chkshsgr.c warn-shsgr tryshsgr.c ipme.h ipme.c \
trysalen.c maildir.5 maildir.h maildir.c tcp-environ.5
	shar -m `cat FILES` > shar
	chmod 400 shar

sig.a: \
makelib sig_alarm.o sig_block.o sig_catch.o sig_pause.o sig_pipe.o \
sig_child.o sig_hup.o sig_term.o sig_bug.o sig_misc.o
	./makelib sig.a sig_alarm.o sig_block.o sig_catch.o \
	sig_pause.o sig_pipe.o sig_child.o sig_hup.o sig_term.o \
	sig_bug.o sig_misc.o

sig_alarm.o: \
compile sig_alarm.c sig_alarm.c sig.h sig_alarm.c
	./compile sig_alarm.c

sig_block.o: \
compile sig_block.c sig_block.c sig.h sig_block.c hassgprm.h \
sig_block.c
	./compile sig_block.c

sig_bug.o: \
compile sig_bug.c sig_bug.c sig.h sig_bug.c
	./compile sig_bug.c

sig_catch.o: \
compile sig_catch.c sig_catch.c sig.h sig_catch.c hassgact.h \
sig_catch.c
	./compile sig_catch.c

sig_child.o: \
compile sig_child.c sig_child.c sig.h sig_child.c
	./compile sig_child.c

sig_hup.o: \
compile sig_hup.c sig_hup.c sig.h sig_hup.c
	./compile sig_hup.c

sig_misc.o: \
compile sig_misc.c sig_misc.c sig.h sig_misc.c
	./compile sig_misc.c

sig_pause.o: \
compile sig_pause.c sig_pause.c sig.h sig_pause.c hassgprm.h \
sig_pause.c
	./compile sig_pause.c

sig_pipe.o: \
compile sig_pipe.c sig_pipe.c sig.h sig_pipe.c
	./compile sig_pipe.c

sig_term.o: \
compile sig_term.c sig_term.c sig.h sig_term.c
	./compile sig_term.c

slurpclose.o: \
compile slurpclose.c stralloc.h gen_alloc.h stralloc.h slurpclose.c \
readwrite.h slurpclose.c slurpclose.h slurpclose.c
	./compile slurpclose.c

socket.lib: \
trylsock.c compile load
	( ( ./compile trylsock.c && \
	./load trylsock -lsocket -lnsl ) >/dev/null 2>&1 \
	&& echo -lsocket -lnsl || exit 0 ) > socket.lib
	rm -f trylsock.o trylsock

spawn.o: \
compile chkspawn spawn.c spawn.c spawn.c sig.h spawn.c wait.h spawn.c \
substdio.h spawn.c byte.h spawn.c str.h spawn.c stralloc.h \
gen_alloc.h stralloc.h spawn.c select.h select.h select.h select.h \
spawn.c exit.h spawn.c coe.h spawn.c open.h spawn.c error.h spawn.c \
auto_qmail.h spawn.c auto_uids.h spawn.c auto_spawn.h spawn.c
	./chkspawn
	./compile spawn.c

splogger: \
load splogger.o substdio.a error.a str.a fs.a syslog.lib
	./load splogger substdio.a error.a str.a fs.a  `cat \
	syslog.lib`

splogger.0: \
splogger.8
	nroff -man splogger.8 > splogger.0

splogger.o: \
compile splogger.c splogger.c splogger.c splogger.c error.h \
splogger.c substdio.h splogger.c subfd.h substdio.h substdio.h \
subfd.h splogger.c exit.h splogger.c str.h splogger.c scan.h \
splogger.c fmt.h splogger.c
	./compile splogger.c

str.a: \
makelib str_len.o str_diff.o str_diffn.o str_cpy.o str_chr.o \
str_rchr.o str_start.o byte_chr.o byte_rchr.o byte_diff.o byte_copy.o \
byte_cr.o byte_zero.o
	./makelib str.a str_len.o str_diff.o str_diffn.o str_cpy.o \
	str_chr.o str_rchr.o str_start.o byte_chr.o byte_rchr.o \
	byte_diff.o byte_copy.o byte_cr.o byte_zero.o

str_chr.o: \
compile str_chr.c str.h str_chr.c
	./compile str_chr.c

str_cpy.o: \
compile str_cpy.c str.h str_cpy.c
	./compile str_cpy.c

str_diff.o: \
compile str_diff.c str.h str_diff.c
	./compile str_diff.c

str_diffn.o: \
compile str_diffn.c str.h str_diffn.c
	./compile str_diffn.c

str_len.o: \
compile str_len.c str.h str_len.c
	./compile str_len.c

str_rchr.o: \
compile str_rchr.c str.h str_rchr.c
	./compile str_rchr.c

str_start.o: \
compile str_start.c str.h str_start.c
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
compile stralloc_arts.c byte.h stralloc_arts.c str.h stralloc_arts.c \
stralloc.h gen_alloc.h stralloc.h stralloc_arts.c
	./compile stralloc_arts.c

stralloc_cat.o: \
compile stralloc_cat.c byte.h stralloc_cat.c stralloc.h gen_alloc.h \
stralloc.h stralloc_cat.c
	./compile stralloc_cat.c

stralloc_catb.o: \
compile stralloc_catb.c stralloc.h gen_alloc.h stralloc.h \
stralloc_catb.c byte.h stralloc_catb.c
	./compile stralloc_catb.c

stralloc_cats.o: \
compile stralloc_cats.c byte.h stralloc_cats.c str.h stralloc_cats.c \
stralloc.h gen_alloc.h stralloc.h stralloc_cats.c
	./compile stralloc_cats.c

stralloc_copy.o: \
compile stralloc_copy.c byte.h stralloc_copy.c stralloc.h gen_alloc.h \
stralloc.h stralloc_copy.c
	./compile stralloc_copy.c

stralloc_eady.o: \
compile stralloc_eady.c alloc.h stralloc_eady.c stralloc.h \
gen_alloc.h stralloc.h stralloc_eady.c gen_allocdefs.h \
gen_allocdefs.h gen_allocdefs.h stralloc_eady.c
	./compile stralloc_eady.c

stralloc_opyb.o: \
compile stralloc_opyb.c stralloc.h gen_alloc.h stralloc.h \
stralloc_opyb.c byte.h stralloc_opyb.c
	./compile stralloc_opyb.c

stralloc_opys.o: \
compile stralloc_opys.c byte.h stralloc_opys.c str.h stralloc_opys.c \
stralloc.h gen_alloc.h stralloc.h stralloc_opys.c
	./compile stralloc_opys.c

stralloc_pend.o: \
compile stralloc_pend.c alloc.h stralloc_pend.c stralloc.h \
gen_alloc.h stralloc.h stralloc_pend.c gen_allocdefs.h \
gen_allocdefs.h gen_allocdefs.h stralloc_pend.c
	./compile stralloc_pend.c

strerr.a: \
makelib strerr_sys.o strerr_die.o
	./makelib strerr.a strerr_sys.o strerr_die.o

strerr_die.o: \
compile strerr_die.c substdio.h strerr_die.c subfd.h substdio.h \
substdio.h subfd.h strerr_die.c exit.h strerr_die.c strerr.h \
strerr_die.c
	./compile strerr_die.c

strerr_sys.o: \
compile strerr_sys.c error.h strerr_sys.c strerr.h strerr_sys.c
	./compile strerr_sys.c

subfderr.o: \
compile subfderr.c readwrite.h subfderr.c substdio.h subfderr.c \
subfd.h substdio.h substdio.h subfd.h subfderr.c
	./compile subfderr.c

subfdin.o: \
compile subfdin.c readwrite.h subfdin.c substdio.h subfdin.c subfd.h \
substdio.h substdio.h subfd.h subfdin.c
	./compile subfdin.c

subfdins.o: \
compile subfdins.c readwrite.h subfdins.c substdio.h subfdins.c \
subfd.h substdio.h substdio.h subfd.h subfdins.c
	./compile subfdins.c

subfdout.o: \
compile subfdout.c readwrite.h subfdout.c substdio.h subfdout.c \
subfd.h substdio.h substdio.h subfd.h subfdout.c
	./compile subfdout.c

subfdouts.o: \
compile subfdouts.c readwrite.h subfdouts.c substdio.h subfdouts.c \
subfd.h substdio.h substdio.h subfd.h subfdouts.c
	./compile subfdouts.c

subgetopt.o: \
compile subgetopt.c subgetopt.h subgetopt.h subgetopt.c
	./compile subgetopt.c

substdi.o: \
compile substdi.c substdio.h substdi.c byte.h substdi.c error.h \
substdi.c
	./compile substdi.c

substdio.a: \
makelib substdio.o substdi.o substdo.o subfderr.o subfdout.o \
subfdouts.o subfdin.o subfdins.o substdio_copy.o
	./makelib substdio.a substdio.o substdi.o substdo.o \
	subfderr.o subfdout.o subfdouts.o subfdin.o subfdins.o \
	substdio_copy.o

substdio.o: \
compile substdio.c substdio.h substdio.c
	./compile substdio.c

substdio_copy.o: \
compile substdio_copy.c substdio.h substdio_copy.c
	./compile substdio_copy.c

substdo.o: \
compile substdo.c substdio.h substdo.c str.h substdo.c byte.h \
substdo.c error.h substdo.c
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
compile tcp-env.c tcp-env.c tcp-env.c tcp-env.c tcp-env.c sig.h \
tcp-env.c stralloc.h gen_alloc.h stralloc.h tcp-env.c str.h tcp-env.c \
env.h tcp-env.c fmt.h tcp-env.c scan.h tcp-env.c subgetopt.h \
tcp-env.c ip.h tcp-env.c dns.h tcp-env.c byte.h tcp-env.c \
remoteinfo.h tcp-env.c exit.h tcp-env.c case.h tcp-env.c
	./compile tcp-env.c

tcp-environ.0: \
tcp-environ.5
	nroff -man tcp-environ.5 > tcp-environ.0

tcpto.o: \
compile tcpto.c tcpto.h tcpto.c open.h tcpto.c lock.h tcpto.c seek.h \
tcpto.c now.h datetime.h now.h tcpto.c ip.h tcpto.c byte.h tcpto.c \
datetime.h datetime.h tcpto.c readwrite.h tcpto.c
	./compile tcpto.c

tcpto_clean.o: \
compile tcpto_clean.c tcpto.h tcpto_clean.c open.h tcpto_clean.c \
substdio.h tcpto_clean.c readwrite.h tcpto_clean.c
	./compile tcpto_clean.c

timeoutconn.o: \
compile timeoutconn.c timeoutconn.c timeoutconn.c timeoutconn.c \
timeoutconn.c ndelay.h timeoutconn.c select.h select.h select.h \
select.h timeoutconn.c error.h timeoutconn.c readwrite.h \
timeoutconn.c ip.h timeoutconn.c byte.h timeoutconn.c timeoutconn.h \
timeoutconn.c
	./compile timeoutconn.c

timeoutread.o: \
compile timeoutread.c timeoutread.h timeoutread.c select.h select.h \
select.h select.h timeoutread.c error.h timeoutread.c readwrite.h \
timeoutread.c
	./compile timeoutread.c

timeoutwrite.o: \
compile timeoutwrite.c timeoutwrite.h timeoutwrite.c select.h \
select.h select.h select.h timeoutwrite.c error.h timeoutwrite.c \
readwrite.h timeoutwrite.c
	./compile timeoutwrite.c

token822.o: \
compile token822.c stralloc.h gen_alloc.h stralloc.h token822.c \
alloc.h token822.c str.h token822.c token822.h gen_alloc.h token822.h \
token822.c gen_allocdefs.h gen_allocdefs.h gen_allocdefs.h token822.c
	./compile token822.c

trigger.o: \
compile trigger.c select.h select.h select.h select.h trigger.c \
open.h trigger.c trigger.h trigger.c hasnpbg1.h trigger.c
	./compile trigger.c

triggerpull.o: \
compile triggerpull.c ndelay.h triggerpull.c open.h triggerpull.c \
triggerpull.h triggerpull.c
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
compile wait_nohang.c wait_nohang.c wait_nohang.c haswaitp.h \
wait_nohang.c
	./compile wait_nohang.c

wait_pid.o: \
compile wait_pid.c wait_pid.c wait_pid.c error.h wait_pid.c
	./compile wait_pid.c
