#include "case.h"

void case_lowers(s)
char *s;
{
  unsigned char x;
  while (x = *s) {
    x -= 'A';
    if (x <= 'Z' - 'A') *s = x + 'a';
    ++s;
  }
}
