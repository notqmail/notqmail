#include "subfd.h"
#include "substdio.h"
#include "ip.h"
#include "ipme.h"
#include "exit.h"

char temp[IPFMT];

void main()
{
 int j;
 switch(ipme_init())
  {
   case 0: substdio_putsflush(subfderr,"out of memory\n"); _exit(111);
   case -1: substdio_putsflush(subfderr,"hard error\n"); _exit(100);
  }
 for (j = 0;j < ipme.len;++j)
  {
   substdio_put(subfdout,temp,ip_fmt(temp,&ipme.ix[j].ip));
   substdio_puts(subfdout,"\n");
  }
 substdio_flush(subfdout);
 _exit(0);
}
