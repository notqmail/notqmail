/*
 * $Log: caltime.h,v $
 * Revision 1.3  2004-10-09 23:20:26+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:57:47+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef CALTIME_H
#define CALTIME_H

#include "caldate.h"
#include "tai.h"

struct caltime
{
	struct caldate  date;
	int             hour;
	int             minute;
	int             second;
	long            offset;
};

void            caltime_tai(struct caltime *, struct tai *);
void            caltime_utc(struct caltime *, struct tai *, int *, int *);

unsigned int    caltime_fmt(char *, struct caltime *);

#endif
