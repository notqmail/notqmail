#ifndef QSUTIL_H
#define QSUTIL_H

#include "deprecated.h"

extern void log1();
extern void qslog2(char *, char *);

#ifdef DEPRECATED_FUNCTIONS_AVAILABLE
static inline void _deprecated_ log2(char *s1, char *s2)
{
  qslog2(s1,s2);
}
#endif

extern void log3();
extern void logsa();
extern void nomem();
extern void pausedir();
extern void logsafe();

#endif
