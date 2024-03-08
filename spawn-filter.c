/*
 * $Id: spawn-filter.c,v 1.87 2024-01-23 01:23:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "fmt.h"
#include "str.h"
#include "strerr.h"
#include "env.h"
#include "substdio.h"
#include "stralloc.h"
#include "error.h"
#include "wait.h"
#include "scan.h"
#include "makeargs.h"
#include "control.h"
#include "report.h"
#include "getDomainToken.h"
#include "qregex.h"
#include "auto_qmail.h"

static int      mkTempFile(int);
static int      run_mailfilter(char *, char *, char *, char **);
static int      check_size(char *);
static void     set_environ(char *, char *, char *, char *);

extern dtype    delivery;
static stralloc sender = { 0 };
static stralloc recipient = { 0 };
static stralloc q = { 0 };

static void
set_environ(char *host, char *ext, char *sender_p, char *recipient_p)
{
	if (ext && !env_put2("_EXT", ext))
		report(111, "spawn-filter: out of memory", ". (#4.3.0)", 0, 0, 0, 0);
	if (!env_put2("DOMAIN", host) ||
			!env_put2("_SENDER", sender_p) ||
			!env_put2("_RECIPIENT", recipient_p))
		report(111, "spawn-filter: out of memory", ". (#4.3.0)", 0, 0, 0, 0);
	return;
}

static int
run_mailfilter(char *domain, char *ext, char *mailprog, char **argv)
{
	char            strnum[FMT_ULONG];
	pid_t           filt_pid;
	int             pipefd[2], pipefe[2];
	int             wstat, filt_exitcode, e, len = 0;
	char           *filterargs;
	static stralloc filterdefs = { 0 };
	static char     errstr[1024];
	char            inbuf[1024];
	char            ch;
	static substdio errbuf;

	if (!(filterargs = env_get("FILTERARGS"))) {
		if (control_readfile(&filterdefs, "control/filterargs", 0) == -1)
			report(111, "spawn-filter: Unable to read filterargs: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		filterargs = getDomainToken(domain, &filterdefs);
	}
	if (!filterargs) {
		/*- Avoid loop if program(s) defined by FILTERARGS call qmail-inject, etc */
		if (!env_unset("FILTERARGS") || !env_unset("SPAMFILTER") ||
				!env_unset("QMAILREMOTE") || !env_unset("QMAILLOCAL"))
			report(111, "spawn-filter: out of memory", ". (#4.3.0)", 0, 0, 0, 0);
		execv(mailprog, argv); /*- do the delivery (qmail-local/qmail-remote) */
		report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
		_exit(111); /*- To make compiler happy */
	}
	if (pipe(pipefd) == -1)
		report(111, "spawn-filter: Trouble creating pipes: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (pipe(pipefe) == -1)
		report(111, "spawn-filter: Trouble creating pipes: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	switch ((filt_pid = fork()))
	{
	case -1:
		report(111, "spawn-filter: Trouble creating child filter: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	case 0: /*- Filter Program */
		set_environ(domain, ext, sender.s, recipient.s);
		/*- Mail content read from fd 0 */
		if (mkTempFile(0))
			report(111, "spawn-filter: lseek error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- stdout will go here */
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		/*- stderr will go here */
		if (dup2(pipefe[1], 2) == -1 || close(pipefe[0]) == -1)
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (pipefe[1] != 2)
			close(pipefe[1]);
		/*- Avoid loop if program(s) defined by FILTERARGS call qmail-inject, etc */
		if (!env_unset("FILTERARGS") || !env_unset("SPAMFILTER"))
			report(111, "spawn-filter: out of memory", ". (#4.3.0)", 0, 0, 0, 0);
		execl("/bin/sh", "sh", "-c", filterargs, (char *) 0);
		report(111, "spawn-filter: could not exec /bin/sh: ",  filterargs, ": ", error_str(errno), ". (#4.3.0)", 0);
	default:
		close(pipefe[1]);
		close(pipefd[1]);
		if (dup2(pipefd[0], 0)) {
			e = errno;
			close(pipefd[0]);
			close(pipefe[0]);
			wait_pid(&wstat, filt_pid);
			errno = e;
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		}
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (mkTempFile(0)) {
			e = errno;
			close(0);
			close(pipefe[0]);
			wait_pid(&wstat, filt_pid);
			errno = e;
			report(111, "spawn-filter: lseek error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		}
		break;
	}
	/*- Process message if exit code is 0, bounce if 100 */
	if (wait_pid(&wstat, filt_pid) != filt_pid) {
		e = errno;
		close(0);
		close(pipefe[0]);
		errno = e;
		report(111, "spawn-filter: waitpid surprise: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	}
	if (wait_crashed(wstat)) {
		e = errno;
		close(0);
		close(pipefe[0]);
		errno = e;
		report(111, "spawn-filter: filter crashed: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	}
	switch (filt_exitcode = wait_exitcode(wstat))
	{
	case 0:
		execv(mailprog, argv); /*- do the delivery (qmail-local/qmail-remote) */
		report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
	case 2:
		report(0, "blackholed", filterargs, 0, 0, 0, 0); /*- Blackhole */
	case 100:
		report(100, "Mail Rejected by ", filterargs, " (#5.7.1)", 0, 0, 0);
	default:
		e = errno;
		substdio_fdbuf(&errbuf, read, pipefe[0], inbuf, sizeof(inbuf));
		for (len = 0; substdio_bget(&errbuf, &ch, 1) && len < (sizeof(errstr) - 1); len++)
			errstr[len] = ch;
		errstr[len] = 0;
		strnum[fmt_ulong(strnum, filt_exitcode)] = 0;
		errno = e;
		report(111, filterargs, ": (spawn-filter) exit code: ", strnum, *errstr ? ": " : 0, *errstr ? errstr : 0, ". (#4.3.0)");
	}
	/*- Not reached */
	return(111);
}

static int
mkTempFile(int seekfd)
{
	char            inbuf[2048], outbuf[2048], strnum[FMT_ULONG];
	char           *tmpdir;
	static stralloc tmpFile = {0};
	struct substdio _ssin;
	struct substdio _ssout;
	int             fd;

	if (lseek(seekfd, 0, SEEK_SET) == 0)
		return (0);
	if (errno == EBADF) {
		strnum[fmt_ulong(strnum, seekfd)] = 0;
		report(111, "spawn-filter: fd ", strnum, ": ", error_str(errno), ". (#4.3.0)", 0);
	}
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir) ||
			!stralloc_cats(&tmpFile, "/qmailFilterXXX") ||
			!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())) ||
			!stralloc_0(&tmpFile))
		report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		report(111, "spawn-filter: ", tmpFile.s, ": ", error_str(errno), ". (#4.3.0)", 0);
	unlink(tmpFile.s);
	substdio_fdbuf(&_ssout, write, fd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&_ssin, read, seekfd, inbuf, sizeof(inbuf));
	switch (substdio_copy(&_ssout, &_ssin))
	{
	case -2: /*- read error */
		report(111, "spawn-filter: read error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	case -3: /*- write error */
		report(111, "spawn-filter: write error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	}
	if (substdio_flush(&_ssout) == -1)
		report(111, "spawn-filter: write error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (dup2(fd, seekfd) == -1)
		report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (lseek(seekfd, 0, SEEK_SET) != 0)
		report(111, "spawn-filter: lseek: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	return (0);
}

static int
check_size(char *size)
{
	char           *x;
	unsigned long   databytes = -1, msgsize;

	if (!(x = env_get("DATABYTES"))) {
		if (control_readulong(&databytes, "control/databytes") == -1)
			report(111, "spawn-filter: Unable to read databytes: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	} else
		scan_ulong(x, &databytes);
	if (databytes == -1)
		return (0);
	scan_ulong(size, &msgsize);
	if (msgsize > databytes)
		return(1);
	else
		return(0);
}

static char *setup_qargs(char t)
{
	static char    *qargs;

	if (!qargs)
		qargs= env_get(t == 'l' ? "QLOCAL" : "QREMOTE");
	if (!qargs) {
		if (!stralloc_copys(&q, auto_qmail) ||
				!stralloc_catb(&q, t == 'l' ? "/bin/qmail-local" : "/bin/qmail-remote", t == 'l' ? 16 : 17) ||
				!stralloc_0(&q))
			report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
		qargs = q.s;
	}
	return qargs;
}

int
main(int argc, char **argv)
{
	char           *ptr, *mailprog, *domain, *size = "0", *ext;
	char            sizebuf[FMT_ULONG];
	int             at, len, i;
	struct stat     statbuf;
	stralloc        defaultdomain = { 0 };

	len = str_len(argv[0]);
	for (ptr = argv[0] + len;*ptr != '/' && ptr != argv[0];ptr--);
	if (*ptr && *ptr == '/')
		ptr++;
	ptr += 6;
	if (*ptr == 'l') { /*- qmail-local Filter */
		/*
		 * usage: qmail-local [ -nN ] user homedir local dash ext domain sender defaultdelivery
		 */
		delivery = local_delivery;
		if (argc != 10 && argc != 9)
			report(100, "spawn-filter: incorrect usage for qmail-local. (#4.3.0)", 0, 0, 0, 0, 0);
		if (!str_diff(argv[1], "-n") || !str_diff(argv[1], "-N") || !str_diff(argv[1], "--")) {
			i = 8;
			ext = argv[6];
		} else {
			i = 7;
			ext = argv[5];
		}
		if (env_get("MATCH_SENDER_DOMAIN")) {
			at = str_rchr(argv[i], '@');
			if (argv[i][at] && argv[i][at + 1])
				domain = argv[i] + at + 1;
			else {
				if (!(domain = env_get("DEFAULT_DOMAIN"))) {
					if (control_readfile(&defaultdomain, "control/defaultdomain", 0) == -1)
						report(111, "spawn-filter: Unable to read defaultdomain: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
					domain = defaultdomain.s;
				}
			}
		} else /*- default for qmail-local is to match on recipient domain */
			domain = argv[i - 1]; /*- recipient domain */
		if (!env_unset("QMAILREMOTE"))
			report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
		/*- sender */
		if (!stralloc_copys(&sender, i == 8 ? argv[8] : argv[7]) || !stralloc_0(&sender))
			report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
		/*- recipient */
		if (*ext) { /*- EXT */
			if (!stralloc_copys(&recipient, ext))
				report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
		} else /*- user */
		if (!stralloc_copys(&recipient, i == 8 ? argv[2] : argv[1]) ||
				!stralloc_cats(&recipient, "@") ||
				!stralloc_cats(&recipient, i == 8 ? argv[7] : argv[6]) ||
				!stralloc_0(&recipient))
			report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
	} else
	if (*ptr == 'r') { /*- qmail-remote Filter */
		/*
		 * usage: qmail-remote host sender recip
		 */
		delivery = remote_delivery;
		if (argc != 4)
			report(100, "spawn-filter: incorrect usage for qmail-remote. (#4.3.0)", 0, 0, 0, 0, 0);
		if (env_get("MATCH_RECIPIENT_DOMAIN"))
			domain = argv[1];
		else { /*- default for qmail-remote is to match on sender domain */
			at = str_rchr(argv[2], '@');
			if (argv[2][at] && argv[2][at + 1])
				domain = argv[2] + at + 1;
			else {
				if (!(domain = env_get("DEFAULT_DOMAIN"))) {
					if (control_readfile(&defaultdomain, "control/defaultdomain", 0) == -1)
						report(111, "spawn-filter: Unable to read defaultdomain: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
					domain = defaultdomain.s;
				}
			}
		}
		ext = argv[3];
		if (!env_unset("QMAILLOCAL"))
			report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
		/*- sender */
		if (!stralloc_copys(&sender, argv[2]) || !stralloc_0(&sender))
			report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
		/*- recipient */
		if (!stralloc_copys(&recipient, argv[5]) || !stralloc_0(&recipient))
			report(111, "spawn-filter: out of memory. (#4.3.0)", 0, 0, 0, 0, 0);
	} else {
		report(111, "spawn-filter: Incorrect usage. ", argv[0], " (#4.3.0)", 0, 0, 0);
		_exit(111);
	}
	if (chdir(auto_qmail) == -1)
		report(111, "spawn-filter: Unable to switch to ", auto_qmail, ": ", error_str(errno), ". (#4.3.0)", 0);

	mailprog = setup_qargs(*ptr); /*- take care not to modify ptr before this line */

	if (!fstat(0, &statbuf)) {
		sizebuf[fmt_ulong(sizebuf, statbuf.st_size)] = 0;
		size = sizebuf;
	} else
		size = "0";
	/*- DATABYTES Check */
	if (check_size(size))
		report(100, "sorry, that message size exceeds my databytes limit (#5.3.4)", 0, 0, 0, 0, 0);
	run_mailfilter(domain, ext, mailprog, argv);
	report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
	_exit(111);
	/*- Not reached */
	return(0);
}

#ifndef lint
void
getversion_qmail_spawn_filter_c()
{
	static char    *x = "$Id: spawn-filter.c,v 1.87 2024-01-23 01:23:36+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidreporth;
	x = sccsidgetdomainth;
	x = sccsidmakeargsh;
	x++;
}
#endif

/*
 * $Log: spawn-filter.c,v $
 * Revision 1.87  2024-01-23 01:23:36+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.86  2024-01-09 12:38:29+05:30  Cprogrammer
 * display filter used for mail rejected message
 *
 * Revision 1.85  2023-12-05 23:19:48+05:30  Cprogrammer
 * setup qmail-local, qmail-remote arguments after envrules
 *
 * Revision 1.84  2023-10-04 23:19:41+05:30  Cprogrammer
 * use env variable QLOCAL, QREMOTE to execute alternate qmail-local, qmail-remote
 *
 * Revision 1.83  2022-03-05 13:39:33+05:30  Cprogrammer
 * use auto_prefix/bin for qmail-inject, auto_prefix/sbin for qmail-local, qmail-remote paths
 *
 * Revision 1.82  2021-08-28 23:08:25+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.81  2021-08-28 21:48:46+05:30  Cprogrammer
 * match sender domain for remote delivery
 * match recipient domain for local delivery
 * allow overrides with MATCH_SENDER_DOMAIN, MATCH_RECIPIENT_DOMAIN
 *
 * Revision 1.80  2021-06-15 12:21:12+05:30  Cprogrammer
 * moved makeargs.h to libqmail
 *
 * Revision 1.79  2021-06-01 01:55:17+05:30  Cprogrammer
 * removed rate limit code, added to qmail-send, slowq-send
 *
 * Revision 1.78  2021-05-26 10:47:10+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.77  2021-05-26 07:38:04+05:30  Cprogrammer
 * moved getDomainToken() to getDomainToken.c
 *
 * Revision 1.76  2021-05-23 07:12:36+05:30  Cprogrammer
 * moved report() to report.c
 * moved rate functions to get_rate.c
 *
 * Revision 1.75  2021-01-28 18:15:34+05:30  Cprogrammer
 * fixed indentation
 *
 * Revision 1.74  2020-12-08 14:15:41+05:30  Cprogrammer
 * save original errno
 *
 * Revision 1.73  2020-11-26 22:23:03+05:30  Cprogrammer
 * indicate the filter name in report to qmail-rspawn
 *
 * Revision 1.72  2020-11-01 23:11:56+05:30  Cprogrammer
 * unset FILTERARGS, SPAMFILTER, QMAILLOCAL, QMAILREMOTE before calling qmail-local, qmail-remote
 *
 * Revision 1.71  2020-05-11 10:59:51+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.70  2020-04-08 15:59:13+05:30  Cprogrammer
 * fixed spamignore control file not being read
 *
 * Revision 1.69  2020-04-01 16:14:54+05:30  Cprogrammer
 * added header for makeargs() function
 *
 * Revision 1.68  2019-09-30 22:59:06+05:30  Cprogrammer
 * use sh as argv0 instead of IndiMailfilter
 *
 * Revision 1.67  2019-07-18 10:48:31+05:30  Cprogrammer
 * use strerr_die?x macro instead of strerr_die() function
 *
 * Revision 1.66  2018-01-31 12:08:27+05:30  Cprogrammer
 * moved qmail-local, qmail-remote to sbin
 *
 * Revision 1.65  2016-06-05 13:22:05+05:30  Cprogrammer
 * fixed stupid error message
 *
 * Revision 1.64  2014-03-26 15:32:26+05:30  Cprogrammer
 * report deliveries blackholed by filters in delivery log
 *
 * Revision 1.63  2014-03-12 15:36:37+05:30  Cprogrammer
 * define REG_NOERROR for OSX / Systems with REG_NOERROR undefined
 *
 * Revision 1.62  2014-03-07 02:07:42+05:30  Cprogrammer
 * do not abort if regcomp() fails
 *
 * Revision 1.61  2014-03-04 02:41:38+05:30  Cprogrammer
 * fix BUG by doing chdir back to auto_qmail
 * ability to have regular expressions on rate control
 * ability to have global definition for rate control
 *
 * Revision 1.60  2014-01-22 15:43:29+05:30  Cprogrammer
 * apply envrules for RATELIMIT_DIR
 *
 * Revision 1.59  2013-09-06 13:58:23+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.58  2013-09-05 09:20:05+05:30  Cprogrammer
 * changed variables to double
 *
 * Revision 1.57  2013-08-29 18:27:15+05:30  Cprogrammer
 * switched to switch statement
 *
 * Revision 1.56  2013-08-27 09:42:18+05:30  Cprogrammer
 * added rate limiting by domain
 *
 * Revision 1.55  2011-06-09 21:28:11+05:30  Cprogrammer
 * blackhole mails if filter program exits 2
 *
 * Revision 1.54  2011-02-08 22:17:37+05:30  Cprogrammer
 * added missing unset of QMAILLOCAL when executing qmail-remote
 *
 * Revision 1.53  2011-01-08 16:41:27+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.52  2010-07-10 10:43:09+05:30  Cprogrammer
 * fixed matching of local/remote directives in filterargs control file
 *
 * Revision 1.51  2010-07-10 09:36:11+05:30  Cprogrammer
 * standardized environment variables set for filters
 *
 * Revision 1.50  2010-07-09 08:22:42+05:30  Cprogrammer
 * implemented sender based envrules using control file fromd.envrules
 *
 * Revision 1.49  2009-11-09 20:32:52+05:30  Cprogrammer
 * Use control file queue_base to process multiple indimail queues
 *
 * Revision 1.48  2009-09-08 13:22:28+05:30  Cprogrammer
 * removed dependency of indimail on spam filtering
 *
 * Revision 1.47  2009-05-01 10:43:40+05:30  Cprogrammer
 * added errstr argument to envrules(), address_match()
 *
 * Revision 1.46  2009-04-29 21:03:40+05:30  Cprogrammer
 * check address_match() for failure
 *
 * Revision 1.45  2009-04-29 14:18:50+05:30  Cprogrammer
 * conditional declaration of spf_fn
 *
 * Revision 1.44  2009-04-29 09:01:03+05:30  Cprogrammer
 * spamignore can be a cdb file
 *
 * Revision 1.43  2009-04-29 08:24:37+05:30  Cprogrammer
 * change for address_match() function
 *
 * Revision 1.42  2009-04-19 13:40:07+05:30  Cprogrammer
 * set environment variable DOMAIN for use in programs called as FILTERS
 *
 * Revision 1.41  2009-04-03 11:42:48+05:30  Cprogrammer
 * create pipe for error messages
 *
 * Revision 1.40  2009-04-02 15:17:54+05:30  Cprogrammer
 * unset QMAILLOCAL in qmail-remote and unset QMAILREMOTE in qmail-local
 *
 * Revision 1.39  2008-06-12 08:40:55+05:30  Cprogrammer
 * added rulesfile argument
 *
 * Revision 1.38  2008-05-25 17:16:43+05:30  Cprogrammer
 * made message more readable by adding a blank space
 *
 * Revision 1.37  2007-12-20 13:51:54+05:30  Cprogrammer
 * avoid loops with FILTERARGS, SPAMFILTERARGS
 * removed compiler warning
 *
 * Revision 1.36  2006-06-07 14:11:28+05:30  Cprogrammer
 * added SPAMEXT, SPAMHOST, SPAMSENDER, QQEH environment variable
 * unset FILTERARGS before calling filters
 *
 * Revision 1.35  2006-01-22 10:14:45+05:30  Cprogrammer
 * BUG fix for spam mails wrongly getting blackholed
 *
 * Revision 1.34  2005-08-23 17:36:48+05:30  Cprogrammer
 * gcc 4 compliance
 * delete sender in spam notification
 *
 * Revision 1.33  2005-04-02 19:07:47+05:30  Cprogrammer
 * use internal wildmat version
 *
 * Revision 1.32  2004-11-22 19:50:53+05:30  Cprogrammer
 * include regex.h after sys/types.h to avoid compilation prob on RH 7.3
 *
 * Revision 1.31  2004-10-22 20:30:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.30  2004-10-21 21:56:21+05:30  Cprogrammer
 * change for two additional arguments to strerr_die()
 *
 * Revision 1.29  2004-10-11 14:06:14+05:30  Cprogrammer
 * use control_readulong instead of control_readint
 *
 * Revision 1.28  2004-09-22 23:14:20+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.27  2004-09-08 10:54:49+05:30  Cprogrammer
 * incorrect exit code in report() function for remote
 * mails. Caused qmail-rspawn to report "Unable to run qmail-remote"
 *
 * Revision 1.26  2004-07-17 21:23:31+05:30  Cprogrammer
 * change qqeh code in qmail-remote
 *
 * Revision 1.25  2004-07-15 23:40:46+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.24  2004-07-02 16:15:25+05:30  Cprogrammer
 * override control files rejectspam, spamredirect by
 * environment variables REJECTSPAM and SPAMREDIRECT
 * allow patterns in domain specification in the control files
 * spamfilterargs, filterargs, rejectspam and spamredirect
 *
 * Revision 1.23  2004-06-03 22:58:34+05:30  Cprogrammer
 * fixed compilation problem without indimail
 *
 * Revision 1.22  2004-05-23 22:18:17+05:30  Cprogrammer
 * added envrules filename as argument
 *
 * Revision 1.21  2004-05-19 23:15:07+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.20  2004-05-12 22:37:47+05:30  Cprogrammer
 * added check DATALIMIT check
 *
 * Revision 1.19  2004-05-03 22:17:36+05:30  Cprogrammer
 * use QUEUE_BASE instead of auto_qmail
 *
 * Revision 1.18  2004-02-13 14:51:24+05:30  Cprogrammer
 * added envrules
 *
 * Revision 1.17  2004-01-20 06:56:56+05:30  Cprogrammer
 * unset FILTERARGS for notifications
 *
 * Revision 1.16  2004-01-20 01:52:08+05:30  Cprogrammer
 * report string length corrected
 *
 * Revision 1.15  2004-01-10 09:44:36+05:30  Cprogrammer
 * added comment for exit codes of bogofilter
 *
 * Revision 1.14  2004-01-08 00:32:49+05:30  Cprogrammer
 * use TMPDIR environment variable for temporary directory
 * send spam reports to central spam logger
 *
 * Revision 1.13  2003-12-30 00:44:42+05:30  Cprogrammer
 * set argv[0] from spamfilterprog
 *
 * Revision 1.12  2003-12-22 18:34:25+05:30  Cprogrammer
 * replaced spfcheck() with address_match()
 *
 * Revision 1.11  2003-12-20 01:35:06+05:30  Cprogrammer
 * added wait_pid to prevent zombies
 *
 * Revision 1.10  2003-12-17 23:33:39+05:30  Cprogrammer
 * improved logic for getting remote/local tokens
 *
 * Revision 1.9  2003-12-16 10:38:24+05:30  Cprogrammer
 * fixed incorrect address being returned if filterargs contained local: or
 * remote: directives
 *
 * Revision 1.8  2003-12-15 20:46:19+05:30  Cprogrammer
 * added case 100 to bounce mail
 *
 * Revision 1.7  2003-12-15 13:51:44+05:30  Cprogrammer
 * code to run additional filters using /bin/sh
 *
 * Revision 1.6  2003-12-14 11:36:18+05:30  Cprogrammer
 * added option to blackhole spammers
 *
 * Revision 1.5  2003-12-13 21:08:46+05:30  Cprogrammer
 * extensive rewrite
 * common report() function for local/remote mails to report errors
 *
 * Revision 1.4  2003-12-12 20:20:55+05:30  Cprogrammer
 * use -a option to prevent using header addresses
 *
 * Revision 1.3  2003-12-09 23:37:16+05:30  Cprogrammer
 * change for spawn-filter to be called as qmail-local or qmail-remote
 *
 * Revision 1.2  2003-12-08 23:48:23+05:30  Cprogrammer
 * new function getDomainToken() to retrieve domain specific values
 * read rejectspam and spamredirect only if SPAMEXITCODE is set
 *
 * Revision 1.1  2003-12-07 13:02:00+05:30  Cprogrammer
 * Initial revision
 *
 */
