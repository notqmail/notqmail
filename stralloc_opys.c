#include "byte.h"
#include "str.h"
#include "stralloc.h"

int stralloc_copys(sa,s)
stralloc *sa;
char *s;
{
  return stralloc_copyb(sa,s,str_len(s));
}
