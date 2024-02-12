/*-
 * $Id: parse_env.c,v 1.1 2023-01-13 12:14:53+05:30 Cprogrammer Exp mbhangui $
 */
#include <ctype.h>
#include "env.h"
#include "parse_env.h"

int
parse_env(char *envStrings)
{
	char           *ptr1, *ptr2, *ptr3, *ptr4;

	for (ptr2 = ptr1 = envStrings;*ptr1;ptr1++) {
		if (*ptr1 == ',') {
			/*
			 * Allow ',' in environment variable if escaped
			 * by '\' character
			 */
			if (ptr1 != envStrings && *(ptr1 - 1) == '\\') {
				for (ptr3 = ptr1 - 1, ptr4 = ptr1; *ptr3; *ptr3++ = *ptr4++);
				continue;
			}
			*ptr1 = 0;
			/*- envar=, - Unset the environment variable */
			if (ptr1 != envStrings && *(ptr1 - 1) == '=') {
				*(ptr1 - 1) = 0;
				if (*ptr2 && !env_unset(ptr2))
					return (1);
			} else { /*- envar=val, - Set the environment variable */
				while (isspace(*ptr2))
					ptr2++;
				if (*ptr2 && !env_put(ptr2))
					return (1);
			}
			ptr2 = ptr1 + 1;
		}
	}
	/*- envar=, */
	if (ptr1 != envStrings && *(ptr1 - 1) == '=') {
		*(ptr1 - 1) = 0;
		if (*ptr2 && !env_unset(ptr2))
			return (1);
	} else { /*- envar=val, */
		while (isspace(*ptr2))
			ptr2++;
		if (*ptr2 && !env_put(ptr2))
			return (1);
	}
	return (0);
}

/*-
 * $Log: parse_env.c,v $
 * Revision 1.1  2023-01-13 12:14:53+05:30  Cprogrammer
 * Initial revision
 *
 */
