/*
 * $Log: getDomainToken.c,v $
 * Revision 1.6  2024-01-05 00:33:59+05:30  Cprogrammer
 * return correct domain when local or remote deliver is not set
 *
 * Revision 1.5  2023-10-30 16:21:01+05:30  Cprogrammer
 * use value of QREGEX to to use matchregex() or wildmat_internal()
 * restore original environment on no match when processing dkimkeys
 *
 * Revision 1.4  2023-02-01 18:14:02+05:30  Cprogrammer
 * added feature to set environment variables in dkimkeys facilitating multi-signature generation with mixed encryption methods
 *
 * Revision 1.3  2021-08-28 23:05:10+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.2  2021-05-29 23:37:58+05:30  Cprogrammer
 * refactored for slowq-send
 *
 * Revision 1.1  2021-05-26 05:20:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stddef.h>
#include <regex.h>
#include "str.h"
#include "stralloc.h"
#include "scan.h"
#include "env.h"
#include "wildmat.h"
#include "getDomainToken.h"
#include "parse_env.h"

#define REGCOMP(X,Y)    regcomp(&X, Y, REG_EXTENDED|REG_ICASE)
#define REGEXEC(X,Y)    regexec(&X, Y, (size_t) 0, (regmatch_t *) 0, (int) 0)
#ifndef REG_NOERROR
#define REG_NOERROR 0
#endif

dtype           delivery;

/*
 * read control file filterargs which can be one of the three
 * 1. domain:local:command with args
 * 2. domain:remote:command with args
 * 3. domain:command with args
 * 4. domain:local:command with args:env1, env2, ...
 * 5. domain:remote:command with args:env1, env2, ...
 * 6. domain:command with args:env1, env2, ...
 * here domain can be a wildcard or a regular expression if
 * QREGEX environment variable is defined
 */
char           *
getDomainToken(char *domain, stralloc *sa)
{
	regex_t         qreg;
	int             len, n, retval, use_regex;
	char           *ptr, *p1, *p2, *x;
	char          **orig_env;

	for (len = 0, ptr = sa->s;len < sa->len;) {
		len += ((n = str_len(ptr)) + 1);
		for (p1 = ptr;*p1 && *p1 != ':';p1++);
		if (*p1 == ':') {
			*p1 = 0; /*- NULL at first ':' */
			/*- check for env strings after command */
			if (!str_diffn(p1 + 1, "remote:", 7))
				for (p2 = p1 + 8; *p2 && *p2 != ':'; p2++);
			else
			if (!str_diffn(p1 + 1, "local:", 6))
				for (p2 = p1 + 7; *p2 && *p2 != ':'; p2++);
			else
				for (p2 = p1 + 1; *p2 && *p2 != ':'; p2++);
			if (*p2 == ':') {
				*p2 = 0;
				parse_env(p2 + 1);
				orig_env = environ; /* save environ to be restored later in case of non-match */
			} else
				orig_env = NULL;
			if ((x = env_get("QREGEX")))
				scan_int(x, &use_regex);
			else
				use_regex = 0;
			/*- build the regex */
			if ((retval = str_diff(ptr, domain))) {
				if (use_regex) {
					if ((retval = REGCOMP(qreg, ptr)) == 0)
						retval = (REGEXEC(qreg, domain) == REG_NOMATCH ? 1 : REG_NOERROR);
					regfree(&qreg);
				} else
					retval = !wildmat_internal(domain, ptr);
			}
			*p1 = ':';
			if (!retval) { /*- match occurred for domain or wildcard */
				/* check for local/remote directives */
				if (delivery == remote_delivery) { /*- domain:remote:command - remote delivery */
					if (!str_diffn(p1 + 1, "remote:", 7))
						return (p1 + 8);
					else
					if (!str_diffn(p1 + 1, "local:", 6)) {
						ptr = sa->s + len;
						continue; /*- skip local directives for remote mails */
					} else
						return (p1 + 1); /*- domain:command */
				} else
				if (delivery == local_delivery) { /*- domain:local:command - local delivery */
					if (!str_diffn(p1 + 1, "local:", 6))
						return (p1 + 7);
					else
					if (!str_diffn(p1 + 1, "remote:", 7)) {
						ptr = sa->s + len;
						continue; /*- skip remote directives for local mails */
					} else
						return (p1 + 1); /*- domain:command */
				} else
				if (delivery == local_or_remote) { /*- local/remote delivery */
					if (!str_diffn(p1 + 1, "local:", 6))
						return (p1 + 7);
					else
					if (!str_diffn(p1 + 1, "remote:", 7))
						return (p1 + 8);
					else
						return (p1 + 1); /*- domain:command */
				} else
				if (str_diffn(p1 + 1, "local:", 6) && str_diffn(p1 + 1, "remote:", 7))
					return (p1 + 1); /*- domain:command */
			} else
			if (orig_env)
				environ = orig_env;
		}
		ptr = sa->s + len;
	} /*- for (len = 0, ptr = sa->s;len < sa->len;) */
	return ((char *) 0);
}

void
getversion_getdomaintoke_c()
{
	static char    *x = "$Id: getDomainToken.c,v 1.6 2024-01-05 00:33:59+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidwildmath;
	x = sccsidgetdomainth;
	x++;
}
