#include "case.h"

void case_lowerb(s,len)
char *s;
unsigned int len;
{
  unsigned char x;
  while (len > 0) {
    --len;
    x = *s - 'A';
    if (x <= 'Z' - 'A') *s = x + 'a';
    ++s;
  }
}
