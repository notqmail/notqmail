#ifndef CDBMSS_H
#define CDBMSS_H

#include "cdbmake.h"
#include "substdio.h"

struct cdbmss {
  char ssbuf[1024];
  struct cdbmake cdbm;
  substdio ss;
  char packbuf[8];
  uint32 pos;
  int fd;
} ;

extern int cdbmss_start();
extern int cdbmss_add();
extern int cdbmss_finish();

#endif
