#include "substdio.h"
#include "subfd.h"
#include "exit.h"
#include "dns.h"
#include "dnsdoe.h"

void dnsdoe(r)
int r;
{
 switch (r)
  {
   case DNS_HARD: substdio_putsflush(subfderr,"hard error\n"); _exit(100);
   case DNS_SOFT: substdio_putsflush(subfderr,"soft error\n"); _exit(111);
   case DNS_MEM: substdio_putsflush(subfderr,"out of memory\n"); _exit(111);
  }
}
