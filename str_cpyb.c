/*
 * $Log: str_cpyb.c,v $
 * Revision 1.2  2004-10-22 20:30:54+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-08-15 19:52:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

unsigned int
str_copyb(s, t, max)
	register char  *s;
	register char  *t;
	unsigned int    max;
{
	register int    len;

	len = 0;
	while (max-- > 0)
	{
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
	}
	return len;
}

void
getversion_str_cpyb_c()
{
	static char    *x = "$Id: str_cpyb.c,v 1.2 2004-10-22 20:30:54+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
