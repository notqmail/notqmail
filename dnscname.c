#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "dns.h"
#include "dnsdoe.h"
#include "readwrite.h"
#include "exit.h"

stralloc sa = {0};

void main(argc,argv)
int argc;
char **argv;
{
 if (!argv[1]) _exit(100);

 if (!stralloc_copys(&sa,argv[1]))
  { substdio_putsflush(subfderr,"out of memory\n"); _exit(111); }

 dns_init(0);
 dnsdoe(dns_cname(&sa));
 substdio_putflush(subfdout,sa.s,sa.len);
 substdio_putsflush(subfdout,"\n");
 _exit(0);
}
