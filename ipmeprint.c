#include "subfd.h"
#include "substdio.h"
#include "ip.h"
#include "ipme.h"

char temp[IPFMT];

int main(void)
{
 int j;
 switch(ipme_init())
  {
   case 0: substdio_putsflush(subfderr,"out of memory\n"); return 111;
   case -1: substdio_putsflush(subfderr,"hard error\n"); return 100;
  }
 for (j = 0;j < ipme.len;++j)
  {
   substdio_put(subfdout,temp,ip_fmt(temp,&ipme.ix[j].ip));
   substdio_puts(subfdout,"\n");
  }
 substdio_flush(subfdout);
 return 0;
}
