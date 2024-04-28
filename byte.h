#ifndef BYTE_H
#define BYTE_H

#include <string.h>

static inline unsigned int byte_chr(const void *s, size_t n, int c)
{
  const void *t = memchr(s, c, n);
  if (!t)
    return n;
  return t - s;
}
extern unsigned int byte_rchr(char *s, size_t n, int c);
#define byte_copy(to,n,from) memcpy(to,from,n)
extern void byte_copyr(char *to, size_t n, const char *from);
extern void byte_zero(void *m, size_t n);

#define byte_equal(s,n,t) (memcmp((s),(t),(n)) == 0)

#endif
