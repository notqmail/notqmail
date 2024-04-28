#include "byte.h"
#include "hasbzero.h"
#ifdef HAS_EXPLICIT_BZERO
#include <string.h>
#endif

void byte_zero(void *m, size_t n)
{
#ifdef HAS_EXPLICIT_BZERO
  explicit_bzero(m, n);
#else
  char *s;
  for (;;) {
    if (!n) break; *s++ = 0; --n;
  }
#endif
}
