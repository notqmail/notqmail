#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"
#include "ipalloc.h"
#include "exit.h"

char temp[IPFMT];

stralloc sa = {0};
ipalloc ia = {0};

void main(argc,argv)
int argc;
char **argv;
{
 int j;

 if (!argv[1]) _exit(100);

 if (!stralloc_copys(&sa,argv[1]))
  { substdio_putsflush(subfderr,"out of memory\n"); _exit(111); }

 dns_init(0);
 dnsdoe(dns_ip(&ia,&sa));
 for (j = 0;j < ia.len;++j)
  {
   substdio_put(subfdout,temp,ip_fmt(temp,&ia.ix[j].ip));
   substdio_putsflush(subfdout,"\n");
  }
 _exit(0);
}
