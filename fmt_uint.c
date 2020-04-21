#include "fmt.h"

unsigned int fmt_uint(char *s, unsigned int u)
{
  register unsigned long l; l = u; return fmt_ulong(s,l);
}
