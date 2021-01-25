#include "fmt.h"

unsigned int fmt_uint(char *s, unsigned int u)
{
  unsigned long l; l = u; return fmt_ulong(s,l);
}
