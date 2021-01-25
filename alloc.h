#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

#include "deprecated.h"

#define alloc(x) malloc(x)
#define alloc_free(x) free(x)

#ifdef DEPRECATED_FUNCTIONS_AVAILABLE
static inline int _deprecated_ alloc_re(void **x, unsigned int m, unsigned int n)
{
  void *y = realloc(*x, n);
  (void)m;
  if (y != NULL)
    *x = y;
  return !!y;
}
#endif

#endif
