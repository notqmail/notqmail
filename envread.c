#include "env.h"
#include "str.h"

extern /*@null@*/char *env_get(s)
char *s;
{
  int i;
  unsigned int slen;
  char *envi;
 
  slen = str_len(s);
  for (i = 0;envi = environ[i];++i)
    if ((!str_diffn(s,envi,slen)) && (envi[slen] == '='))
      return envi + slen + 1;
  return 0;
}

extern char *env_pick()
{
  return environ[0];
}

extern char *env_findeq(s)
char *s;
{
  for (;*s;++s)
    if (*s == '=')
      return s;
  return 0;
}
