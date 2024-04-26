#include "byte.h"

void byte_copyr(char *to, unsigned int n, const char *from)
{
  to += n;
  from += n;
  for (;;) {
    if (!n) return; *--to = *--from; --n;
  }
}
