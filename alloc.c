#include <stdlib.h>
#include "alloc.h"
#include "error.h"

#define ALIGNMENT 16 /* XXX: assuming that this alignment is enough */
#define SPACE 4096 /* must be multiple of ALIGNMENT */

typedef union { char irrelevant[ALIGNMENT]; double d; } aligned;
static aligned realspace[SPACE / ALIGNMENT];
#define space ((char *) realspace)
static unsigned int avail = SPACE; /* multiple of ALIGNMENT; 0<=avail<=SPACE */

static char *m_alloc(unsigned int n)
{
  char *x = malloc(n);
  if (!x) errno = error_nomem;
  return x;
}

/*@null@*//*@out@*/char *alloc(n)
unsigned int n;
{
  if (n >= SPACE)
    return m_alloc(n);
  /* Round it up to the next multiple of alignment. Could overflow if n is
   * close to 2**32, but by the check above this is already ruled out. */
  n = ALIGNMENT + n - (n & (ALIGNMENT - 1));
  if (n <= avail) { avail -= n; return space + avail; }
  return m_alloc(n);
}

void alloc_free(x)
char *x;
{
  if (x >= space)
    if (x < space + SPACE)
      return; /* XXX: assuming that pointers are flat */
  free(x);
}
