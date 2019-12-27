#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

#define alloc(x) malloc(x)
#define alloc_free(x) free(x)
static inline int alloc_re(char **x, unsigned int m, unsigned int n)
{
  char *y = realloc(*x, n);
  (void)m;
  if (y != NULL)
    *x = y;
  return !!y;
}

#endif
