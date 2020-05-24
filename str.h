#ifndef STR_H
#define STR_H

#include <string.h>

#define str_copy(s,t) strcpy((s),(t))
#define str_diff(s,t) strcmp((s),(t))
#define str_diffn(s,t,len) strncmp((s),(t),(len))
#define str_len(s) strlen((s))
extern unsigned int str_chr(const char *s, int c);

static inline unsigned int str_rchr(const char *s, int c)
{
  const char *t = strrchr(s, c);
  if (!t)
    return strlen(s);
  return t - s;
}

static inline int str_start(const char *s, const char *t)
{
  return strncmp(s, t, strlen(t)) == 0;
}

#define str_equal(s,t) (strcmp((s),(t)) == 0)

#endif
