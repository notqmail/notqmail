#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "str.h"
#include "scan.h"
#include "dns.h"
#include "dnsdoe.h"
#include "strsalloc.h"
#include "exit.h"

strsalloc ssa = {0};
stralloc sa = {0};

void main(argc,argv)
int argc;
char **argv;
{
 int j;

 if (!argv[1]) _exit(100);

 if (!stralloc_copys(&sa, argv[1]))
  { substdio_putsflush(subfderr,"out of memory\n"); _exit(111); }
 dns_init(0);
 dnsdoe(dns_txt(&ssa,&sa));
 for (j = 0;j < ssa.len;++j)
  {
   substdio_put(subfdout,ssa.sa[j].s,ssa.sa[j].len);
   substdio_putsflush(subfdout,"\n");
  }
 _exit(0);
}
