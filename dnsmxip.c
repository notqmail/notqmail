#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "fmt.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"
#include "ipalloc.h"
#include "now.h"
#include "exit.h"

char temp[IPFMT + FMT_ULONG];

stralloc sa = {0};
ipalloc ia = {0};

void main(argc,argv)
int argc;
char **argv;
{
 int j;
 unsigned long r;

 if (!argv[1]) _exit(100);

 if (!stralloc_copys(&sa,argv[1]))
  { substdio_putsflush(subfderr,"out of memory\n"); _exit(111); }

 r = now() + getpid();
 dns_init(0);
 dnsdoe(dns_mxip(&ia,&sa,r));
 for (j = 0;j < ia.len;++j)
  {
   substdio_put(subfdout,temp,ip_fmt(temp,&ia.ix[j].ip));
   substdio_puts(subfdout," ");
   substdio_put(subfdout,temp,fmt_ulong(temp,(unsigned long) ia.ix[j].pref));
   substdio_putsflush(subfdout,"\n");
  }
 _exit(0);
}
