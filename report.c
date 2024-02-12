/*
 * $Log: report.c,v $
 * Revision 1.6  2023-12-06 17:02:13+05:30  Cprogrammer
 * added comment on report format for qmail-rspawn
 *
 * Revision 1.5  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.4  2021-08-28 23:07:59+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.3  2021-05-30 00:16:51+05:30  Cprogrammer
 * renamed local to local_delivery
 *
 * Revision 1.2  2021-05-26 07:36:54+05:30  Cprogrammer
 * fixed extra colon char in error messages
 *
 * Revision 1.1  2021-05-23 06:35:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "strerr.h"
#include "report.h"
#include "getDomainToken.h"

extern dtype    delivery;

/*-
 * qmail-rspawn doesn't use exit code of qmail-remote. It needs a report in
 * the following format
 * "[r,h,s]recipient_report\0[K,Z,D]message_report\0"
 *
 * recipient_report start with one of the letters r, h, s
 * as below
 * r - Recipient report: acceptance.
 * s - Recipient report: temporary rejection.
 * h - Recipient report: permanent rejection.
 *
 * message_report start with one of the letters K, Z, D
 * as below
 * K - Message report: success.
 * Z - Message report: temporary failure.
 * D - Message report: permanent failure.
 *
 * Examples of qmail-remote report
 *
 * Success
 * "rFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "KHost example.com accepted message\0\n"
 *
 * temp failure
 * "sFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "ZTemporary failure accepting message\0\n"
 *
 * perm failure
 * "hFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "Dexample.org does not like recipient\0\n"
 *
 * qmail-lspawn uses the exit code of qmail-local
 * 0   - Success
 * 111 - Temporary failure
 * 100 - Permanent failure
 */
void
report(int errCode, char *s1, char *s2, char *s3, char *s4, char *s5, char *s6)
{
	if (delivery == local_delivery) /*- strerr_die does not return */
		strerr_die6x(errCode, s1, s2, s3, s4, s5, s6);
	if (!errCode) { /*- should never happen */
		if (substdio_put(subfdoutsmall, "r\0Kspawn accepted message.\n\0", 28) == -1)
			_exit(111);
		if (s1) {
			if (substdio_puts(subfdoutsmall, s1) == -1 ||
					substdio_put(subfdoutsmall, "\n", 1) == -1)
				_exit(111);
		} else
		if (substdio_put(subfdoutsmall, "spawn said: 250 ok notification queued\n\0", 41) == -1)
			_exit(111);
	} else {
		/*- h - hard, s - soft */
		if (substdio_put(subfdoutsmall, errCode == 111 ? "s" : "h", 1) == -1)
			_exit(111);
		if (s1 && substdio_puts(subfdoutsmall, s1) == -1)
			_exit(111);
		if (s2 && substdio_puts(subfdoutsmall, s2) == -1)
			_exit(111);
		if (s3 && substdio_puts(subfdoutsmall, s3) == -1)
			_exit(111);
		if (s4 && substdio_puts(subfdoutsmall, s4) == -1)
			_exit(111);
		if (s5 && substdio_puts(subfdoutsmall, s5) == -1)
			_exit(111);
		if (s6 && substdio_puts(subfdoutsmall, s6) == -1)
			_exit(111);
		if (substdio_put(subfdoutsmall, "\0", 1) == -1)
			_exit(111);
		if (substdio_puts(subfdoutsmall,
			errCode == 111 ?  "Zspawn said: Message deferred" : "Dspawn said: Giving up on filter\n") == -1)
			_exit(111);
		if (substdio_put(subfdoutsmall, "\0", 1) == -1)
			_exit(111);
	}
	substdio_flush(subfdoutsmall);
	/*- For qmail-rspawn to stop complaining unable to run qmail-remote */
	_exit(0);
}

void
getversion_report_c()
{
	static char    *x = "$Id: report.c,v 1.6 2023-12-06 17:02:13+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidreporth;
	x = sccsidgetdomainth;
	x++;
}
