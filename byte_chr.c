#include "byte.h"

unsigned int byte_chr(char *s, unsigned int n, int c)
{
  char ch;
  char *t;

  ch = c;
  t = s;
  for (;;) {
    if (!n) break; if (*t == ch) break; ++t; --n;
    if (!n) break; if (*t == ch) break; ++t; --n;
    if (!n) break; if (*t == ch) break; ++t; --n;
    if (!n) break; if (*t == ch) break; ++t; --n;
  }
  return t - s;
}
