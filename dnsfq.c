#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"
#include "ipalloc.h"
#include "exit.h"

stralloc sa = {0};
ipalloc ia = {0};

void main(argc,argv)
int argc;
char **argv;
{
 if (!argv[1]) _exit(100);

 if (!stralloc_copys(&sa,argv[1]))
  { substdio_putsflush(subfderr,"out of memory\n"); _exit(111); }

 dns_init(1);
 dnsdoe(dns_ip(&ia,&sa));
 if (ia.len <= 0)
  {
   substdio_putsflush(subfderr,"no IP addresses\n"); _exit(100);
  }
 dnsdoe(dns_ptr(&sa,&ia.ix[0].ip));
 substdio_putflush(subfdout,sa.s,sa.len);
 substdio_putsflush(subfdout,"\n");
 _exit(0);
}
