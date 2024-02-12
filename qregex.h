/*
 * $Log: qregex.h,v $
 * Revision 1.3  2004-09-21 23:49:02+05:30  Cprogrammer
 * added matchregex() and setdotChar()
 *
 * Revision 1.2  2003-12-22 18:35:26+05:30  Cprogrammer
 * added address_match() function
 *
 * Revision 1.1  2003-12-20 13:17:45+05:30  Cprogrammer
 * Initial revision
 *
 */
/*
 * simple header file for the matchregex prototype 
 */
#ifndef _QREGEX_H_
#define _QREGEX_H_
#include "constmap.h"
#include "stralloc.h"

int             address_match(stralloc *, int, stralloc *, struct constmap *, int, stralloc *);
int             matchregex(char *, char *);
void            setdotChar(char);
#endif
