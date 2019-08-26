#include "fmt.h"

/* writes ulong u in hex to char *s, does not NULL-terminate */
unsigned int fmt_xlong(char *s, unsigned long u)
{
 unsigned int len;
 unsigned long q;
 unsigned long c;
 len = 1; q = u;
 while (q > 15) { ++len; q /= 16; }
 if (s)
  {
   s += len;
   do { c = u & 15; *--s = (c > 9 ? 'a' - 10 : '0') + c; u /= 16; } while(u);
  }
 return len;
}
