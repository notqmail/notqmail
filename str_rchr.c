#include "str.h"

unsigned int str_rchr(s,c)
register char *s;
int c;
{
  register char ch;
  register char *t;
  register char *u;

  ch = c;
  t = s;
  u = 0;
  for (;;) {
    if (!*t) break; if (*t == ch) u = t; ++t;
    if (!*t) break; if (*t == ch) u = t; ++t;
    if (!*t) break; if (*t == ch) u = t; ++t;
    if (!*t) break; if (*t == ch) u = t; ++t;
  }
  if (!u) u = t;
  return u - s;
}
