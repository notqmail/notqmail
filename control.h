#ifndef CONTROL_H
#define CONTROL_H

#include "stralloc.h"

extern int control_init(void);
extern int control_readline(stralloc *sa, const char *fn);
extern int control_rldef(stralloc *sa, const char *fn, int flagme, const char *def);
extern int control_readint(int *i, const char *fn);
extern int control_readfile(stralloc *sa, const char *fn, int flagme);

#endif
