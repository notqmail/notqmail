/*
 * $Log: makeargs.h,v $
 * Revision 1.1  2021-06-09 21:27:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef MAKEARGS_H
#define MAKEARGS_H

#ifndef	lint
static char     sccsidmakeargsh[] = "$Id: makeargs.h,v 1.1 2021-06-09 21:27:06+05:30 Cprogrammer Exp mbhangui $";
#endif

char          **makeargs(char *);
void            free_makeargs(char **);

#endif
