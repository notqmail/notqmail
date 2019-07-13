#include "str.h"

unsigned int str_copyb(s,t,max)
register char *s;
register char *t;
unsigned int max;
{
  register int len;

  len = 0;
  while (max-- > 0) {
    if (!(*s = *t)) return len; ++s; ++t; ++len;
    if (!(*s = *t)) return len; ++s; ++t; ++len;
    if (!(*s = *t)) return len; ++s; ++t; ++len;
    if (!(*s = *t)) return len; ++s; ++t; ++len;
  }
  return len;
}
