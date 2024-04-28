#ifndef BYTE_H
#define BYTE_H

#include <string.h>

static inline unsigned int byte_chr(const void *s, unsigned int n, int c)
{
  const void *t = memchr(s, c, n);
  if (!t)
    return n;
  return t - s;
}
extern unsigned int byte_rchr();
#define byte_copy(to,n,from) memcpy(to,from,n)
extern void byte_zero();
extern void byte_zero(void *m, unsigned int n);

#define byte_equal(s,n,t) (memcmp((s),(t),(n)) == 0)

#endif
