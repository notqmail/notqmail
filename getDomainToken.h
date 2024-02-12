/*
 * $Log: getDomainToken.h,v $
 * Revision 1.2  2021-08-28 23:03:41+05:30  Cprogrammer
 * added dtype enum from variables.h
 *
 * Revision 1.1  2021-05-26 05:20:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _GETDOMAINTOKEN_H
#define _GETDOMAINTOKEN_H
#include "stralloc.h"

#ifndef	lint
static char     sccsidgetdomainth[] = "$Id: getDomainToken.h,v 1.2 2021-08-28 23:03:41+05:30 Cprogrammer Exp mbhangui $";
#endif

typedef enum {
	unknown,
	local_delivery,
	remote_delivery,
	local_or_remote,
} dtype;

char           *getDomainToken(char *, stralloc *);

#endif
