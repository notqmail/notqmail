#include "str.h"

unsigned int str_chr(s,c)
register char *s;
int c;
{
  register char ch;
  register char *t;

  ch = c;
  t = s;
  for (;;) {
    if (!*t) break; if (*t == ch) break; ++t;
    if (!*t) break; if (*t == ch) break; ++t;
    if (!*t) break; if (*t == ch) break; ++t;
    if (!*t) break; if (*t == ch) break; ++t;
  }
  return t - s;
}
