#ifndef READSUBDIR_H
#define READSUBDIR_H

#include "direntry.h"

typedef struct readsubdir
 {
  DIR *dir;
  int pos;
  char *name;
  void (*pause)();
 }
readsubdir;

extern void readsubdir_init();
extern int readsubdir_next();

#define READSUBDIR_NAMELEN 10

#endif
