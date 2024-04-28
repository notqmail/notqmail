#include "str.h"

#include <string.h>

unsigned int str_chr(const char *s, int c)
{
  const char *r = strchr(s, c);
  if (!r)
    return strlen(s);
  return r - s;
}
