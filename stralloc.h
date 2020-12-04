#ifndef STRALLOC_H
#define STRALLOC_H

#include "gen_alloc.h"
#include <string.h>

GEN_ALLOC_typedef(stralloc,char,s,len,a)

extern int stralloc_ready();
extern int stralloc_readyplus();
extern int stralloc_copy();
extern int stralloc_cat();
extern int stralloc_copyb(stralloc *sa, const char *s, unsigned int n);
extern int stralloc_catb(stralloc *sa, const char *s, unsigned int n);
extern int stralloc_append(); /* beware: this takes a pointer to 1 char */
extern int stralloc_starts(stralloc *sa, const char *s);
static inline int stralloc_cats(stralloc *sa, const char *s)
{
  return stralloc_catb(sa,s,strlen(s));
}
static inline int stralloc_copys(stralloc *sa, const char *s)
{
  return stralloc_copyb(sa,s,strlen(s));
}

#define stralloc_0(sa) stralloc_append(sa,"")

#endif
