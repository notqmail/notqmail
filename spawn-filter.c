#include "fmt.h"
#include "str.h"
#include "strerr.h"
#include "env.h"
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "error.h"
#include "control.h"
#include "wait.h"
#include "auto_qmail.h"
#include <regex.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define REGCOMP(X,Y)    regcomp(&X, Y, REG_EXTENDED|REG_ICASE)
#define REGEXEC(X,Y)    regexec(&X, Y, (size_t) 0, (regmatch_t *) 0, (int) 0)

static int      mkTempFile(int);
static void     report(int, char *, char *, char *, char *, char *, char *);
char           *getDomainToken(char *, stralloc *);
static int      run_mailfilter(char *, char *, char **);

static int      remotE;
stralloc        sender = { 0 };
stralloc        recipient = { 0 };

static void
report(int errCode, char *s1, char *s2, char *s3, char *s4, char *s5, char *s6)
{
	if (!remotE) /*- strerr_die does not return */
		strerr_die(errCode, s1, s2, s3, s4, s5, s6, 0, 0, (struct strerr *) 0);
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
		errCode == 111 ?  "Zspawn-filter said: Message deferred" : "DGiving up on spawn-filter\n") == -1)
		_exit(111);
	if (substdio_put(subfdoutsmall, "\0", 1) == -1)
		_exit(111);
	substdio_flush(subfdoutsmall);
	/*- For qmail-rspawn to stop complaining unable to run qmail-remote */
	_exit(0);
}

void
set_environ(char *host, char *sender, char *recipient)
{
	if (!env_put2("DOMAIN", host)) 
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!env_put2("_SENDER", sender))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!env_put2("_RECIPIENT", recipient))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	return;
}

static int
run_mailfilter(char *domain, char *mailprog, char **argv)
{
	char            strnum[FMT_ULONG];
	pid_t           filt_pid;
	int             pipefd[2], pipefe[2];
	int             wstat, filt_exitcode, len = 0, e;
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
		execv(mailprog, argv);
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
		set_environ(domain, sender.s, recipient.s);
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
		if (!env_unset("FILTERARGS"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		execl("/bin/sh", "IndiMailfilter", "-c", filterargs, (char *) 0);
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
		execv(mailprog, argv);
		report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
	case 100:
		report(100, "Mail Rejected (#5.7.1)", 0, 0, 0, 0, 0);
	default:
		substdio_fdbuf(&errbuf, read, pipefe[0], inbuf, sizeof(inbuf));
		for (len = 0; substdio_bget(&errbuf, &ch, 1) && len < (sizeof(errstr) - 1); len++)
			errstr[len] = ch;
		errstr[len] = 0;
		strnum[fmt_ulong(strnum, filt_exitcode)] = 0;
		report(111, filterargs, ": (spawn-filter) exit code: ", strnum, *errstr ? ": " : 0, *errstr ? errstr : 0, ". (#4.3.0)");
	}
	/*- Not reached */
	return(111);
}

char           *
getDomainToken(char *domain, stralloc *sa)
{
	regex_t         qreg;
	int             len, n, retval;
	char           *ptr, *p;
	char            errbuf[512];

	for (len = 0, ptr = sa->s;len < sa->len;) {
		len += ((n = str_len(ptr)) + 1);
		for (p = ptr;*p && *p != ':';p++);
		if (*p == ':') {
			*p = 0;
			/*- build the regex */
			if ((retval = str_diff(ptr, domain))) {
				if ((retval = REGCOMP(qreg, ptr)) != 0) {
					regerror(retval, &qreg, errbuf, sizeof(errbuf));
					regfree(&qreg);
					report(111, "spawn-filter: ", ptr, ": ", errbuf, ". (#4.3.0)", 0);
				}
				retval = REGEXEC(qreg, domain);
				regfree(&qreg);
			}
			*p = ':';
			if (!retval) {/*- match occurred for domain or wildcard */
				/* check for local/remote directives */
				if (remotE) {
					if (!str_diffn(p + 1, "remote:", 7))
						return (p + 8);
					if (!str_diffn(p + 1, "local:", 6)) {
						ptr = sa->s + len;
						continue; /*- skip local directives for remote mails */
					}
				} else {
					if (!str_diffn(p + 1, "local:", 6))
						return (p + 7);
					if (!str_diffn(p + 1, "remote:", 7)) {
						ptr = sa->s + len;
						continue; /*- skip remote directives for remote mails */
					}
				}
				return (p + 1);
			}
		}
		ptr = sa->s + len;
	}
	return ((char *) 0);
}

int
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
	if (!stralloc_copys(&tmpFile, tmpdir))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!stralloc_cats(&tmpFile, "/qmailFilterXXX"))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!stralloc_0(&tmpFile))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		report(111, "spawn-filter: ", tmpFile.s, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (unlink(tmpFile.s))
		report(111, "spawn-filter: unlink: ", tmpFile.s, ": ", error_str(errno), ". (#4.3.0)", 0);
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

int
main(int argc, char **argv)
{
	char           *ptr, *mailprog, *domain, *ext;
	int             len;

	len = str_len(argv[0]);
	for (ptr = argv[0] + len;*ptr != '/' && ptr != argv[0];ptr--);
	if (*ptr && *ptr == '/')
		ptr++;
	ptr += 6; /*- will be either "local" or "remote" */
	if (*ptr == 'l') { /*- qmail-local Filter */
		mailprog = "bin/qmail-local";
		ext = argv[6];
		domain = argv[7];
		remotE = 0;
		if (!env_unset("QMAILREMOTE"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- sender */
		if (!stralloc_copys(&sender, argv[8]))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&sender))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- recipient */
		if (*ext) { /*- EXT */
			if (!stralloc_copys(&recipient, ext))
				report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		} else /*- user */
		if (!stralloc_copys(&recipient, argv[2]))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_cats(&recipient, "@"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_cats(&recipient, domain))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&recipient))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	} else
	if (*ptr == 'r') { /*- qmail-remote Filter */
		mailprog = "bin/qmail-remote";
		domain = argv[1]; /*- host */
		remotE = 1;
		if (!env_unset("QMAILLOCAL"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- sender */
		if (!stralloc_copys(&sender, argv[2]))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&sender))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- recipient */
		if (!stralloc_copys(&recipient, argv[3]))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&recipient))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	} else {
		report(111, "spawn-filter: Incorrect usage. ", argv[0], " (#4.3.0)", 0, 0, 0);
		_exit(111);
	}
	if (chdir(auto_qmail) == -1)
		report(111, "spawn-filter: Unable to switch to ", auto_qmail, ": ", error_str(errno), ". (#4.3.0)", 0);
	run_mailfilter(domain, mailprog, argv);
	report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
	/*- Not reached */
	return(0);
}
