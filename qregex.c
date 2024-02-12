/*
 * $Log: qregex.c,v $
 * Revision 1.13  2007-12-20 13:51:04+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.12  2005-08-23 17:41:36+05:30  Cprogrammer
 * regex to be turned on only of QREGEX is defined to non-zero value
 *
 * Revision 1.11  2005-04-02 19:07:25+05:30  Cprogrammer
 * use internal wildmat version
 *
 * Revision 1.10  2005-01-22 00:39:04+05:30  Cprogrammer
 * added missing error handling
 *
 * Revision 1.9  2004-10-22 20:29:45+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.8  2004-09-21 23:48:18+05:30  Cprogrammer
 * made matchregex() visible
 * introduced dotChar (configurable dot char)
 *
 * Revision 1.7  2004-02-05 18:48:48+05:30  Cprogrammer
 * changed curregex to static
 *
 * Revision 1.6  2003-12-23 23:22:53+05:30  Cprogrammer
 * implicitly use wildcard if address starts with '@'
 *
 * Revision 1.5  2003-12-22 18:33:12+05:30  Cprogrammer
 * added address_match()
 *
 * Revision 1.4  2003-12-22 13:21:08+05:30  Cprogrammer
 * added text and pattern as part of error message
 *
 * Revision 1.3  2003-12-22 10:04:04+05:30  Cprogrammer
 * conditional compilation of qregex
 *
 * Revision 1.2  2003-12-21 15:32:18+05:30  Cprogrammer
 * added regerror
 *
 * Revision 1.1  2003-12-20 13:17:16+05:30  Cprogrammer
 * Initial revision
 *
 * qregex (v2)
 * $Id: qregex.c,v 1.13 2007-12-20 13:51:04+05:30 Cprogrammer Stab mbhangui $
 *
 * Author  : Evan Borgstrom (evan at unixpimps dot org)
 * Created : 2001/12/14 23:08:16
 * Modified: $Date: 2007-12-20 13:51:04+05:30 $
 * Revision: $Revision: 1.13 $
 *
 * Do POSIX regex matching on addresses for anti-relay / spam control.
 * It logs to the maillog
 * See the qregex-readme file included with this tarball.
 * If you didn't get this file in a tarball please see the following URL:
 *  http://www.unixpimps.org/software/qregex
 *
 * qregex.c is released under a BSD style copyright.
 * See http://www.unixpimps.org/software/qregex/copyright.html
 *
 * Note: this revision follows the coding guidelines set forth by the rest of
 *       the qmail code and that described at the following URL.
 *       http://cr.yp.to/qmail/guarantee.html
 * 
 */
#include "case.h"
#include "scan.h"
#include "stralloc.h"
#include "constmap.h"
#include "substdio.h"
#include "byte.h"
#include "env.h"
#include <sys/types.h>
#include <regex.h>
#include <unistd.h>

static int      wildmat_match(stralloc *, int, struct constmap *, int, stralloc *);
static int      regex_match(stralloc *, int, stralloc *);
int             wildmat_internal(char *, char *);

static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static char     dotChar = '@';

int
address_match(stralloc *addr, int bhfok, stralloc *bhf,
	struct constmap *mapbhf, int bhpok, stralloc *bhp)
{
	char           *ptr;
	int             x = 0;

	case_lowerb(addr->s, addr->len); /*- convert into lower case */
	if ((ptr = env_get("QREGEX")))
		scan_int(ptr, &x);
	if (ptr && x)
		return (regex_match(addr, bhfok, bhf));
	else
		return (wildmat_match(addr, bhfok, mapbhf, bhpok, bhp));
}

int
matchregex(char *text, char *regex)
{
	regex_t         qreg;
	char            errbuf[512];
	int             retval = 0;

#define REGCOMP(X,Y)    regcomp(&X, Y, REG_EXTENDED|REG_ICASE)
	/*- build the regex */
	if ((retval = REGCOMP(qreg, regex)) != 0)
	{
		regerror(retval, &qreg, errbuf, sizeof(errbuf));
		regfree(&qreg);
		if (substdio_puts(&sserr, text) == -1)
			return (-retval);
		if (substdio_puts(&sserr, ": ") == -1)
			return (-retval);
		if (substdio_puts(&sserr, regex) == -1)
			return (-retval);
		if (substdio_puts(&sserr, ": ") == -1)
			return (-retval);
		if (substdio_puts(&sserr, errbuf) == -1)
			return (-retval);
		if (substdio_puts(&sserr, "\n") == -1)
			return (-retval);
		if (substdio_flush(&sserr) == -1)
			return (-retval);
		return (-retval);
	}
	/*- execute the regex */
#define REGEXEC(X,Y)    regexec(&X, Y, (size_t) 0, (regmatch_t *) 0, (int) 0)
	retval = REGEXEC(qreg, text);
	regfree(&qreg);
	return (retval == REG_NOMATCH ? 0 : 1);
}

static int
wildmat_match(stralloc * addr, int mapfile, struct constmap *ptrmap, int patfile, stralloc *wildcard)
{
	int             i = 0;
	int             j = 0;
	int             k = 0;
	char            subvalue;

	if (mapfile)
	{
		if (constmap(ptrmap, addr->s, addr->len - 1))
			return 1;
		if ((j = byte_rchr(addr->s, addr->len, dotChar)) < addr->len)
		{
			if (constmap(ptrmap, addr->s + j, addr->len - j - 1))
				return 1;
		}
	}
	/*- Include control file control/xxxxpatterns and evaluate with Wildmat check */
	if (patfile && wildcard)
	{
		i = 0;
		for (j = 0; j < wildcard->len; ++j)
		{
			if (!wildcard->s[j])
			{
				subvalue = wildcard->s[i] != '!';
				if (!subvalue)
					i++;
				if ((k != subvalue) && wildmat_internal(addr->s, wildcard->s + i))
					k = subvalue;
				i = j + 1;
			}
		}
		return k;
	}
	return (0);
}

static int
regex_match(stralloc * addr, int mapfile, stralloc *map)
{
	int             i = 0;
	int             j = 0;
	int             k = 0;
	int             negate = 0, match;
	static stralloc curregex = { 0 };

	match = 0;
	if (mapfile)
	{
		while (j < map->len)
		{
			i = j;
			while ((map->s[i] != '\0') && (i < map->len))
				i++;
			if (map->s[j] == '!')
			{
				negate = 1;
				j++;
			}
			if (*(map->s + j) == dotChar)
			{
				if (!stralloc_copys(&curregex, ".*"))
					return(-1);
				if (!stralloc_catb(&curregex, map->s + j, (i - j)))
					return(-1);
			} else
			if (!stralloc_copyb(&curregex, map->s + j, (i - j)))
				return(-1);
			if (!stralloc_0(&curregex))
				return(-1);
			if((k = matchregex(addr->s, curregex.s)) == 1)
			{
				if (negate)
					return(0);
				match = 1;
			}
			j = i + 1;
			negate = 0;
		}
	}
	return (match);
}

void
setdotChar(c)
	char            c;
{
	dotChar = c;
	return;
}

void
getversion_qregex_c()
{
	static char    *x = "$Id: qregex.c,v 1.13 2007-12-20 13:51:04+05:30 Cprogrammer Stab mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#else
	x++;
#endif
}
