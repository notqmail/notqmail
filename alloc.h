#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

#define alloc(x) malloc(x)
#define alloc_free(x) free(x)

/*
 * In-tree code must not call this deprecated function. It's here because
 * some external patches still do. We intend to remove it as soon as is
 * practical.
 */
#ifndef DONT_PROVIDE_OBSOLETE

#if defined(__clang__) || defined(__GNUC__)
#define _deprecated_ __attribute__((deprecated))
#else
#define _deprecated_
#endif

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
