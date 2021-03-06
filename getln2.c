#include "substdio.h"
#include "stralloc.h"
#include "byte.h"
#include "getln.h"

int getln2(substdio *ss, stralloc *sa,
           /*@out@*/char **cont, /*@out@*/unsigned int *clen,
           int sep)
{
  char *x;
  unsigned int i;
 
  if (!stralloc_ready(sa,0)) return -1;
  sa->len = 0;
 
  for (;;) {
    ssize_t n, m;
    n = substdio_feed(ss);
    if (n < 0) return -1;
    if (n == 0) { *clen = 0; return 0; }
    x = substdio_PEEK(ss);
    i = byte_chr(x,n,sep);
    if (i < n) { substdio_SEEK(ss,*clen = i + 1); *cont = x; return 0; }
    if (!stralloc_readyplus(sa,n)) return -1;
    i = sa->len;
    m = substdio_get(ss,sa->s + i,n);
    if (m != -1)
      sa->len = i + m;
  }
}
