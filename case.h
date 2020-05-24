#ifndef CASE_H
#define CASE_H

#include <string.h>
#include <strings.h>

extern void case_lowers(char *s);
extern void case_lowerb(char *s, unsigned int len);
#define case_diffs(s,t) (strcasecmp((s),(t)) != 0)
extern int case_diffb(char *s, unsigned int len, char *t);
extern int case_starts(char *s, char *t);
extern int case_startb();

#define case_equals(s,t) (!case_diffs((s),(t)))

#endif
