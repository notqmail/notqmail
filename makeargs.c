/*
 * $Log: MakeArgs.c,v $
 * Revision 2.8  2007-12-21 14:35:42+05:30  Cprogrammer
 * included env.h to prevent compiler warning
 *
 * Revision 2.7  2007-12-20 12:45:28+05:30  Cprogrammer
 * expand environment variables with '$' sign
 *
 * Revision 2.6  2005-08-23 17:31:28+05:30  Cprogrammer
 * removed sccsid variable
 *
 * Revision 2.5  2005-04-02 19:06:02+05:30  Cprogrammer
 * djb version
 *
 * Revision 2.4  2005-03-30 22:52:47+05:30  Cprogrammer
 * BUG - Incorrect free
 *
 * Revision 2.3  2004-07-12 22:47:58+05:30  Cprogrammer
 * bug fix. Free all allocated members
 *
 * Revision 2.2  2002-12-21 18:21:09+05:30  Cprogrammer
 * added functionality of escaping text via quotes
 *
 * Revision 2.1  2002-08-13 20:35:44+05:30  Cprogrammer
 * addition spaces were not getting skipped
 *
 * Revision 1.2  2002-03-03 17:23:05+05:30  Cprogrammer
 * replaced strcpy with scopy
 *
 * Revision 1.1  2001-12-13 01:46:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
#include "str.h"
#include "stralloc.h"
#include "env.h"
#include <ctype.h>
#include "makeargs.h"

#define isEscape(ch) ((ch) == '"' || (ch) == '\'')

/*
 * function to expand a string into command line
 * arguments. To free memory allocated by this
 * function the following should be done
 *
 * free(argv);
 *
 */
char          **
makeargs(char *cmmd)
{
	char           *ptr, *marker;
	char          **argv;
	int             argc, idx;
	static stralloc sptr = { 0 };

	for (ptr = cmmd;*ptr && isspace((int) *ptr);ptr++);
	idx = str_len(ptr);
	if (!stralloc_copys(&sptr, ptr))
		return((char **) 0);
	if (!stralloc_0(&sptr))
		return((char **) 0);
	/*-
	 * Get the number of arguments by counting
	 * white spaces. Allow escape via the double
	 * quotes character at the first word
	 */
	for (argc = 0, ptr = sptr.s;*ptr;)
	{
		for (;*ptr && isspace((int) *ptr);ptr++);
		if (!*ptr)
			break;
		argc++;
		marker = ptr;
		/*- Move till you hit the next white space */
		for (;*ptr && !isspace((int) *ptr);ptr++)
		{
			/*-
			 * 1. If escape char is encounted skip till you
			 *    hit the terminating escape char
			 * 2. If terminating escape char is missing, come
			 *    back to the start escape char
			 */
			if (ptr == marker && isEscape(*ptr))
			{
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
			}
		} /*- for(;*ptr && !isspace((int) *ptr);ptr++) */
	} /*- for (argc = 0, ptr = sptr.s;*ptr;) */
	/*
	 * Allocate memory to store the arguments
	 * Do not bother extra bytes occupied by
	 * white space characters.
	 */
	if (!(argv = (char **) alloc((argc + 1) * sizeof(char *))))
		return ((char **) 0);
	for (idx = 0, ptr = sptr.s;*ptr;)
	{
		for (;*ptr && isspace((int) *ptr);ptr++)
			*ptr = 0;
		if (!*ptr)
			break;
		if (*ptr == '$')
			argv[idx++] = env_get(ptr + 1);
		else
			argv[idx++] = ptr;
		marker = ptr;
		for (;*ptr && !isspace((int) *ptr);ptr++)
		{
			if (ptr == marker && isEscape(*ptr))
			{
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
				else /*- Remove the quotes */
				{
					argv[idx - 1] += 1;
					*ptr = 0;
				}
			}
		}
	} /*- for (idx = 0, ptr = sptr.s;*ptr;) */
	argv[idx++] = (char *) 0;
	return (argv);
}

void
free_makeargs(char **argv)
{
	alloc_free(argv);
	return;
}

void
getversion_MakeArgs__c()
{
	static char     *x = "$Id: MakeArgs.c,v 2.8 2007-12-21 14:35:42+05:30 Cprogrammer Stab mbhangui $";
	x++;
	x--;
	return;
}
