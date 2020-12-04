#ifndef QSUTIL_H
#define QSUTIL_H

#include "deprecated.h"

extern void log1(const char *);
extern void qslog2(const char *, const char *);

#ifdef DEPRECATED_FUNCTIONS_AVAILABLE
static inline void _deprecated_ log2(const char *s1, const char *s2)
{
  qslog2(s1,s2);
}
#endif

extern void log3(const char *s1, const char *s2, const char *s3);
extern void logsa();
extern void nomem();
extern void pausedir(const char *dir);
extern void logsafe(const char *s);

#endif
