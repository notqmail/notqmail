#include "scan.h"

unsigned int scan_ulong(char *s, unsigned long *u)
{
  unsigned int pos; unsigned long result;
  unsigned long c;
  pos = 0; result = 0;
  while ((c = (unsigned long) (unsigned char) (s[pos] - '0')) < 10)
    { result = result * 10 + c; ++pos; }
  *u = result; return pos;
}
