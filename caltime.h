#ifndef CALTIME_H
#define CALTIME_H

#include "caldate.h"

struct caltime {
  struct caldate date;
  int hour;
  int minute;
  int second;
  long offset;
} ;

extern void caltime_tai(const struct caltime *, struct tai *);
extern void caltime_utc(struct caltime *, const struct tai *, int *, int *);

extern unsigned int caltime_fmt(char *, const struct caltime *);
extern unsigned int caltime_scan(const char *, struct caltime *);

#endif
