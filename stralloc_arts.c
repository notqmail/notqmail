#include "byte.h"
#include "str.h"
#include "stralloc.h"

int stralloc_starts(sa,s)
stralloc *sa;
char *s;
{
  int len;
  len = str_len(s);
  return (sa->len >= len) && byte_equal(s,len,sa->s);
}
