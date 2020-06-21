#include "substdio.h"
#include "subfd.h"
#include "exit.h"
#include "strerr.h"

void strerr_warn(const char *x1, const char *x2, const char *x3,
                 const char *x4, const char *x5, const char *x6,
                 struct strerr *se)
{
  strerr_sysinit();
 
  if (x1) substdio_puts(subfderr,x1);
  if (x2) substdio_puts(subfderr,x2);
  if (x3) substdio_puts(subfderr,x3);
  if (x4) substdio_puts(subfderr,x4);
  if (x5) substdio_puts(subfderr,x5);
  if (x6) substdio_puts(subfderr,x6);
 
  while(se) {
    if (se->x) substdio_puts(subfderr,se->x);
    if (se->y) substdio_puts(subfderr,se->y);
    if (se->z) substdio_puts(subfderr,se->z);
    se = se->who;
  }
 
  substdio_puts(subfderr,"\n");
  substdio_flush(subfderr);
}

void strerr_die(int e, const char *x1, const char *x2, const char *x3,
                const char *x4, const char *x5, const char *x6,
                struct strerr *se)
{
  strerr_warn(x1,x2,x3,x4,x5,x6,se);
  _exit(e);
}
