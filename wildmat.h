/*
 * $Log: wildmat.h,v $
 * Revision 1.1  2021-05-23 06:35:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _WILDMAT_H
#define _WILDMAT_H

#ifndef	lint
static char     sccsidwildmath[] = "$Id: wildmat.h,v 1.1 2021-05-23 06:35:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifndef TRUE
#define TRUE			 1
#endif
#ifndef FALSE
#define FALSE			 0
#endif
#ifndef ABORT
#define ABORT			-1
#endif

/*- What character marks an inverted character class?  */
#define NEGATE_CLASS		'^'
/*- Is "*" a common pattern?  */
#define OPTIMIZE_JUST_STAR
/*- Do tar(1) matching rules, which ignore a trailing slash?  */
#undef MATCH_TAR_PATTERN

int             wildmat_internal(char *, char *);

#endif
