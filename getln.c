#include "substdio.h"
#include "byte.h"
#include "stralloc.h"
#include "getln.h"

int getln(ss,sa,match,sep)
register substdio *ss;
register stralloc *sa;
int *match;
int sep;
{
  char *cont;
  unsigned int clen;

  if (getln2(ss,sa,&cont,&clen,sep) == -1) return -1;
  if (!clen) { *match = 0; return 0; }
  if (!stralloc_catb(sa,cont,clen)) return -1;
  *match = 1;
  return 0;
}
