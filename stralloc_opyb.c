#include "stralloc.h"
#include "byte.h"
#include "error.h"
#include "oflops.h"

int stralloc_copyb(sa,s,n)
stralloc *sa;
char *s;
unsigned int n;
{
  unsigned int i;
  if (__builtin_add_overflow(n, 1, &i)) {
    errno = error_nomem;
    return 0;
  }
  if (!stralloc_ready(sa,i)) return 0;
  byte_copy(sa->s,n,s);
  sa->len = n;
  sa->s[n] = 'Z'; /* ``offensive programming'' */
  return 1;
}
