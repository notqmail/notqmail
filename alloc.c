#include <stdlib.h>
#include "alloc.h"
#include "error.h"
#include "hasbltnoverflow.h"

#define ALIGNMENT 16 /* XXX: assuming that this alignment is enough */
#define SPACE 4096 /* must be multiple of ALIGNMENT */

typedef union { char irrelevant[ALIGNMENT]; double d; } aligned;
static aligned realspace[SPACE / ALIGNMENT];
#define space ((char *) realspace)
static unsigned int avail = SPACE; /* multiple of ALIGNMENT; 0<=avail<=SPACE */

/*@null@*//*@out@*/char *alloc(n)
unsigned int n;
{
  char *x;
  unsigned int m;
#ifdef HAS_BUILTIN_OVERFLOW
  if (__builtin_add_overflow(ALIGNMENT, n - (n & (ALIGNMENT - 1)), &m)) {
#else
  m = n;
  if ((n = ALIGNMENT + n - (n & (ALIGNMENT - 1))) < m) { /*- handle overflow */
#endif
    errno = error_nomem;
    return 0;
  }
  if (n <= avail) { avail -= n; return space + avail; }
  x = malloc(n);
  if (!x) errno = error_nomem;
  return x;
}

void alloc_free(x)
char *x;
{
  if (x >= space)
    if (x < space + SPACE)
      return; /* XXX: assuming that pointers are flat */
  free(x);
}
