#ifndef SPAWN_H
#define SPAWN_H

#include "substdio.h"

extern int truncreport;
extern int spawn(int fdmess, int fdout, char *s, char *r, int at);
extern void report(substdio *ss, int wstat, char *s, int len);
extern void initialize(int argc, char **argv);

#endif
