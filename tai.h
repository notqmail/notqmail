/*
 * $Log: tai.h,v $
 * Revision 1.3  2004-10-11 14:15:10+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-09-19 22:49:23+05:30  Cprogrammer
 * added tai_unix macro
 *
 * Revision 1.1  2004-06-16 01:20:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TAI_H
#define TAI_H

#include "uint64.h"

struct tai
{
	uint64          x;
};

#define tai_unix(t,u) ((void) ((t)->x = 4611686018427387914ULL + (uint64) (u)))
#define tai_approx(t) ((double) ((t)->x))
#define tai_less(t,u) ((t)->x < (u)->x)
#define TAI_PACK 8

void            tai_now(struct tai *);
void            tai_add();
void            tai_sub(struct tai *, struct tai *, struct tai *);
void            tai_pack(char *, struct tai *);
void            tai_unpack(char *, struct tai *);

#endif
