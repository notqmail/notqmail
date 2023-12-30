#include "stralloc.h"
#include "byte.h"
#include "error.h"
#include "oflops.h"

int stralloc_catb(sa,s,n)
stralloc *sa;
char *s;
unsigned int n;
{
  unsigned int i;
  if (!sa->s) return stralloc_copyb(sa,s,n);
  if (__builtin_add_overflow(n, 1, &i)) {
    errno = error_nomem;
    return 0;
  }
  if (!stralloc_readyplus(sa,i)) return 0;
  byte_copy(sa->s + sa->len,n,s);
  sa->len += n;
  sa->s[sa->len] = 'Z'; /* ``offensive programming'' */
  return 1;
}
