#include "fmt.h"

unsigned int fmt_strn(s,t,n)
char *s; char *t; unsigned int n;
{
  unsigned int len;
  char ch;
  len = 0;
  if (s) { while (n-- && (ch = t[len])) s[len++] = ch; }
  else while (n-- && t[len]) len++;
  return len;
}
