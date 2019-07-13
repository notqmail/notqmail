#include "byte.h"

unsigned int byte_rcspn(s,n,c)
register char *s;
register unsigned int n;
register char *c;
{
  unsigned int ret,pos,i;

  for(ret = n,pos = 0;*c;++c) {
    i = byte_rchr(s + pos,n - pos,*c) + pos;
    if (i < n) ret = pos = i;
  }

  return ret;
}

