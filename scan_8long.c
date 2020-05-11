#include "scan.h"

unsigned int scan_8long(char *s, unsigned long *u)
{
  register unsigned int pos; register unsigned long result;
  register unsigned long c;
  pos = 0; result = 0;
  while ((c = (unsigned long) (unsigned char) (s[pos] - '0')) < 8)
    { result = result * 8 + c; ++pos; }
  *u = result; return pos;
}
