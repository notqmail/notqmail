#include "datetime.h"
#include "fmt.h"
#include "date822fmt.h"

static char *montab[12] = {
"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

unsigned int date822fmt(s,dt)
char *s;
struct datetime *dt;
{
  unsigned int i;
  unsigned int len;
  len = 0;
  i = fmt_uint(s,dt->mday); len += i; if (s) s += i;
  i = fmt_str(s," "); len += i; if (s) s += i;
  i = fmt_str(s,montab[dt->mon]); len += i; if (s) s += i;
  i = fmt_str(s," "); len += i; if (s) s += i;
  i = fmt_uint(s,dt->year + 1900); len += i; if (s) s += i;
  i = fmt_str(s," "); len += i; if (s) s += i;
  i = fmt_uint0(s,dt->hour,2); len += i; if (s) s += i;
  i = fmt_str(s,":"); len += i; if (s) s += i;
  i = fmt_uint0(s,dt->min,2); len += i; if (s) s += i;
  i = fmt_str(s,":"); len += i; if (s) s += i;
  i = fmt_uint0(s,dt->sec,2); len += i; if (s) s += i;
  i = fmt_str(s," -0000\n"); len += i; if (s) s += i;
  return len;
}
