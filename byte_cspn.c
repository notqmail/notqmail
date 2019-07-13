#include "byte.h"

unsigned int byte_cspn(s,n,c)
register char *s;
register unsigned int n;
register char *c;
{
  while(*c)
    n = byte_chr(s,n,*c++);
  return n;
}
