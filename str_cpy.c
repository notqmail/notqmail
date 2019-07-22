#include "str.h"

unsigned int str_copy(s,t)
char *s;
char *t;
{
  int len;

  len = 0;
  for (;;) {
    if (!(*s = *t)) return len; ++s; ++t; ++len;
    if (!(*s = *t)) return len; ++s; ++t; ++len;
    if (!(*s = *t)) return len; ++s; ++t; ++len;
    if (!(*s = *t)) return len; ++s; ++t; ++len;
  }
}
