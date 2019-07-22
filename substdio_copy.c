#include "substdio.h"

int substdio_copy(ssout,ssin)
substdio *ssout;
substdio *ssin;
{
  int n;
  char *x;

  for (;;) {
    n = substdio_feed(ssin);
    if (n < 0) return -2;
    if (!n) return 0;
    x = substdio_PEEK(ssin);
    if (substdio_put(ssout,x,n) == -1) return -3;
    substdio_SEEK(ssin,n);
  }
}
