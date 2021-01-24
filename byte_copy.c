#include "byte.h"

void byte_copy(char *to, unsigned int n, char *from)
{
  for (;;) {
    if (!n) return; *to++ = *from++; --n;
  }
}
