#include "ehlo_parse.h"

#include <stdio.h>

int64_t remotesize = -1;

/**
 * @brief parse a string as decimal number
 * @retval 0 the value was a valid number
 *
 * Similar to strtoll(), but with a length limit.
 */
static int nparse(const char *str, size_t slen)
{
  remotesize = 0;

  while (slen) {
    if (*str < '0' || *str > '9')
      return 1;
    remotesize *= 10;
    remotesize += (*str - '0');
    str++;
    slen--;
  }

  return 0;
}

int ehlo_size(const char *ext, size_t extlen)
{
  char *end;

  /* only "SIZE" given: the server supports the extension, but does not give a size limit */
  if (extlen == 4) {
    remotesize = 0;
    return 1;
  }

  if (nparse(ext + 5, extlen - 5)) {
    remotesize = -1;
    return 0;
  }

  return 1;
}
