/* Public domain. */

#include "tai.h"

void tai_unpack(const char *s,struct tai *t)
{
  uint64_t x;
  int i;

  x = (unsigned char) s[0];
  for (i = 1; i < 8; i++) {
    x <<= 8;
    x += (unsigned char) s[i];
  }
  t->x = x;
}
