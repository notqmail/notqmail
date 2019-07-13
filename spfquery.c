#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "alloc.h"
#include "spf.h"
#include "exit.h"

void die(e,s) int e; char *s; { substdio_putsflush(subfderr,s); _exit(e); }
void die_usage() { die(100,"fatal: invalid usage\nusage: spfquery <sender-ip> <sender-helo/ehlo> <envelope-from> [<local rules>] [<best guess rules>]\n"); }
void die_nomem() { die(111,"fatal: out of memory\n"); }

stralloc addr = {0};
stralloc helohost = {0};
char *remoteip;
char *local;

stralloc spflocal = {0};
stralloc spfguess = {0};
stralloc spfexp = {0};

void main(argc,argv)
int argc;
char **argv;
{
	stralloc sa = {0};
	int r;

	if (argc < 4) die_usage();

	remoteip = (char *)strdup(argv[1]);
	local = "localhost";

	if (!stralloc_copys(&helohost, argv[2])) die_nomem();
	if (!stralloc_0(&helohost)) die_nomem();

	if (!stralloc_copys(&addr, argv[3])) die_nomem();
	if (!stralloc_0(&addr)) die_nomem();

	if (argc > 4) {
		if (!stralloc_copys(&spflocal, argv[4])) die_nomem();
		if (spflocal.len && !stralloc_0(&spflocal)) die_nomem();
	}

	if (argc > 5) {
		if (!stralloc_copys(&spfguess, argv[5])) die_nomem();
		if (spfguess.len && !stralloc_0(&spfguess)) die_nomem();
	}

	if (argc > 6) {
		if (!stralloc_copys(&spfexp, argv[6])) die_nomem();
	} else
		if (!stralloc_copys(&spfexp, SPF_DEFEXP)) die_nomem();
	if (spfexp.len && !stralloc_0(&spfexp)) die_nomem();

	dns_init(0);
	r = spfcheck();
	if (r == SPF_NOMEM) die_nomem();

	substdio_puts(subfdout,"result=");
	switch(r) {
		case SPF_OK: substdio_puts(subfdout,"pass"); break;
		case SPF_NONE: substdio_puts(subfdout,"none"); break;
		case SPF_UNKNOWN: substdio_puts(subfdout,"unknown"); break;
		case SPF_NEUTRAL: substdio_puts(subfdout,"neutral"); break;
		case SPF_SOFTFAIL: substdio_puts(subfdout,"softfail"); break;
		case SPF_FAIL: substdio_puts(subfdout,"fail"); break;
		case SPF_ERROR: substdio_puts(subfdout,"error"); break;
	}

	if (r == SPF_FAIL) {
		substdio_puts(subfdout,": ");
		if (!spfexplanation(&sa)) die_nomem();
		substdio_put(subfdout,sa.s,sa.len);
	}

	substdio_putsflush(subfdout,"\n");

	substdio_puts(subfdout,"Received-SPF: ");
	if (!spfinfo(&sa)) die_nomem();
	substdio_put(subfdout,sa.s,sa.len);
	substdio_putsflush(subfdout,"\n");

	_exit(0);
}
