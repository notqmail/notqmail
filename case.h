#ifndef CASE_H
#define CASE_H

#include <string.h>
#include <strings.h>

extern void case_lowers();
extern void case_lowerb();
#define case_diffs(s,t) (strcasecmp((s),(t)) != 0)
extern int case_diffb();
extern int case_starts();
extern int case_startb();

#define case_equals(s,t) (!case_diffs((s),(t)))

#endif
