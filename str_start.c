#include "str.h"

int str_start(s,t)
register char *s;
register char *t;
{
  register char x;

  for (;;) {
    x = *t++; if (!x) return 1; if (x != *s++) return 0;
    x = *t++; if (!x) return 1; if (x != *s++) return 0;
    x = *t++; if (!x) return 1; if (x != *s++) return 0;
    x = *t++; if (!x) return 1; if (x != *s++) return 0;
  }
}
