/*
 * $Log: caldate.h,v $
 * Revision 1.3  2004-10-09 23:20:20+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:57:44+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef CALDATE_H
#define CALDATE_H

struct caldate
{
	long            year;
	int             month;
	int             day;
};

void            caldate_frommjd(struct caldate *, long, int *, int *);
long            caldate_mjd(struct caldate *);
unsigned int    caldate_fmt(char *, struct caldate *);

#endif
