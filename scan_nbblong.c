#include "scan.h"

unsigned int scan_nbblong(s,n,base,bext,u)
char *s; unsigned int n; unsigned int base; unsigned int bext; unsigned long *u;
/* Note that n == 0 means scan forever. Hopefully this is a good choice. */
{
  unsigned int pos; unsigned long result; unsigned long c;
  pos = 0; result = 0;
  while (((c = (unsigned long) (unsigned char) (s[pos] - '0')) < base)
       ||(((c = (unsigned long) (unsigned char) (s[pos] - 'a')) < bext)
        &&(c = c + base))
       ||(((c = (unsigned long) (unsigned char) (s[pos] - 'A')) < bext)
        &&(c = c + base))
        ) /* this gets the job done */
    { result = result * (base + bext) + c; ++pos; if (pos == n) break; }
  *u = result; return pos;
}

unsigned int scan_nbbint(s,n,base,bext,u)
char *s; unsigned int n; unsigned int base; unsigned int bext; unsigned int *u;
{
  unsigned int pos; unsigned long result;
  pos = scan_nbblong(s,n,base,bext,&result);
  *u = result; return pos;
}

unsigned int scan_nbbshort(s,n,base,bext,u)
char *s; unsigned int n; unsigned int base; unsigned int bext; unsigned short *u;
{
  unsigned int pos; unsigned long result;
  pos = scan_nbblong(s,n,base,bext,&result);
  *u = result; return pos;
}
