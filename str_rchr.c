#include "str.h"

unsigned int str_rchr(s,c)
char *s;
int c;
{
  char ch;
  char *t;
  char *u;

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
