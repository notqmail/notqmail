#include "byte.h"

void byte_zero(char *s, unsigned int n)
{
  for (;;) {
    if (!n) break; *s++ = 0; --n;
  }
}
