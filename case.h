#ifndef CASE_H
#define CASE_H

extern void case_lowers();
extern void case_lowerb();
extern int case_diffs();
extern int case_diffb();
extern int case_starts();
extern int case_startb();

#define case_equals(s,t) (!case_diffs((s),(t)))

#endif
