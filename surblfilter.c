/*
 * $Log: surblfilter.c,v $
 * Revision 1.3  2011-07-13 22:11:13+05:30  Cprogrammer
 * skip surblrcpt if QMAILRCPTS is not defined
 *
 * Revision 1.2  2011-07-13 22:02:13+05:30  Cprogrammer
 * added surblrcpt functionality
 *
 * Revision 1.1  2011-07-13 20:56:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef DARWIN
#include <nameser8_compat.h>
#endif
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

#include "alloc.h"
#include "sgetopt.h"
#include "error.h"
#include "scan.h"
#include "str.h"
#include "case.h"
#include "constmap.h"
#include "auto_qmail.h"
#include "stralloc.h"
#include "env.h"
#include "control.h"
#include "strerr.h"
#include "substdio.h"
#include "getln.h"
#include "byte.h"
#include "dns.h"
#include "ip.h"
#include "ipalloc.h"
#include "mess822.h"
#include "base64.h"

#define FATAL "surblfilter: fatal: "

char           *dns_text(char *);

stralloc        line = { 0 };
int             debug = 0, do_text = 0, do_cache = 1;
static int      cachelifetime = 300;
stralloc        whitelist = { 0 };
stralloc        surbldomain = { 0 };

/*- SURBL: RCPT whitelist. */
stralloc        srw = { 0 };
int             srwok = 0;
struct constmap mapsrw;

/*- 2 level tld */
stralloc        l2 = { 0 };
int             l2ok = 0;
struct constmap mapl2;
/*- 3 level tld */
stralloc        l3 = { 0 };
int             l3ok = 0;
struct constmap mapl3;

static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static char     ssdbgbuf[512];
static substdio ssdbg = SUBSTDIO_FDBUF(write, 5, ssdbgbuf, sizeof(ssdbgbuf));

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(&ssout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
print_debug(char *arg1, char *arg2, char *arg3)
{
	if (!debug)
		return;
	if (arg1 && substdio_puts(&ssdbg, arg1) == -1)
		_exit(1);
	if (arg2 && substdio_puts(&ssdbg, arg2) == -1)
		_exit(1);
	if (arg3 && substdio_puts(&ssdbg, arg3) == -1)
		_exit(1);
	if ((arg1 || arg2 || arg3) && substdio_puts(&ssdbg, "\n"))
		_exit(1);
	if (substdio_flush(&ssdbg) == -1)
		_exit(1);
}

void
die_write()
{
	strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(&ssout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
my_error(char *s1, char *s2, int exit_val)
{
	logerr(s1);
	if (s2) {
		logerr(": ");
		logerr(s2);
	}
	if (exit_val > 0) {
		logerr(": ");
		logerr(error_str(errno));
	}
	logerrf("\n");
	_exit(exit_val > 0 ? exit_val : 0 - exit_val);
}

void
die_nomem()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: out of memory\n");
	substdio_flush(&sserr);
	_exit(1);
}

void
die_soft()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: DNS temporary failure\n");
	substdio_flush(&sserr);
	_exit(1);
}

void
die_hard()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: DNS permanent failure\n");
	substdio_flush(&sserr);
	_exit(1);
}

void
die_control()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: unable to read controls\n");
	substdio_flush(&sserr);
	_exit(1);
}

static unsigned short
getshort(unsigned char *cp)
{
	return (cp[0] << 8) | cp[1];
}

/*
 * we always return a null-terminated string which has been malloc'ed.  The string
 * is always in the tag=value form.  If a temporary or permanent error occurs,
 * the string will be exactly "e=perm;" or "e=temp;".
 * Note that it never returns NULL.
 */
char           *
dns_text(char *dn)
{
	u_char          response[PACKETSZ + PACKETSZ + 1];	/* response */
	int             responselen;			/* buffer length */
	int             rc;						/* misc variables */
	int             ancount, qdcount;		/* answer count and query count */
	u_short         type, rdlength;			/* fields of records returned */
	u_char         *eom, *cp;
	u_char          buf[PACKETSZ + PACKETSZ + 1];		/* we're storing a TXT record here, not just a DNAME */
	u_char         *bufptr;

	for (rc = 0, responselen = PACKETSZ;rc < 2;rc++) {
		if ((responselen = res_query(dn, C_IN, T_TXT, response, responselen)) < 0) {
			if (h_errno == TRY_AGAIN)
				return strdup("e=temp;");
			else
				return strdup("e=perm;");
		}
		if (responselen <= PACKETSZ)
			break;
		else
		if (responselen >= (2 * PACKETSZ))
			return strdup("e=perm;");
	}
	qdcount = getshort(response + 4);	/* http://crynwr.com/rfc1035/rfc1035.html#4.1.1. */
	ancount = getshort(response + 6);
	eom = response + responselen;
	cp = response + HFIXEDSZ;
	while (qdcount-- > 0 && cp < eom) {
		rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME);
		if (rc < 0)
			return strdup("e=perm;");
		cp += rc + QFIXEDSZ;
	}
	while (ancount-- > 0 && cp < eom) {
		if ((rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME)) < 0)
			return strdup("e=perm;");
		cp += rc;
		if (cp + RRFIXEDSZ >= eom)
			return strdup("e=perm;");
		type = getshort(cp + 0);	/* http://crynwr.com/rfc1035/rfc1035.html#4.1.3. */
		rdlength = getshort(cp + 8);
		cp += RRFIXEDSZ;
		if (type != T_TXT) {
			cp += rdlength;
			continue;
		}
		bufptr = buf;
		while (rdlength && cp < eom) {
			unsigned int    cnt;

			cnt = *cp++;		/* http://crynwr.com/rfc1035/rfc1035.html#3.3.14. */
			if (bufptr - buf + cnt + 1 >= (2 * PACKETSZ))
				return strdup("e=perm;");
			if (cp + cnt > eom)
				return strdup("e=perm;");
			byte_copy((char *) bufptr, cnt, (char *) cp);
			rdlength -= cnt + 1;
			bufptr += cnt;
			cp += cnt;
			*bufptr = '\0';
		}
		return (char *) strdup((char *) buf);
	}
	return strdup("e=perm;");
}

static char    *
uri_decode(char *str, size_t str_len, char **strend)
{
	size_t          i = 0, j = 0, found;
	int             pasthostname = 0;
	char           *str_bits = "\r\n\t \'\"<>()";

	for (i = 0; i < str_len; i++, j++) {
		if (str[i] == '%' || (!pasthostname && str[i] == '=')) {
			if (i + 2 < str_len) {
				if (isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
					int             c1 = str[i + 1];
					int             c2 = str[i + 2];
					int             num = (	/* first character */
											  ((c1 & 0xF)	/* take right half */
											   +(9 * (c1 >> 6)))	/* add 9 if character is a-f or A-F */
											  <<4	/* pack into the left half of the byte */
						) | (	/* second character */
								(c2 & 0xF)
								+ (9 * (c2 >> 6))
						);		/* leave it as the left half */
					str[j] = tolower(num);
					i += 2;
					continue;
				}
			}
		}
		if (!pasthostname && (str[i] == '?' || str[i] == '/' || str[i] == '\\'))
			pasthostname = 1;
		if (i + 1 < str_len) {
			if (str[i] == '=' && str[i + 1] == '\n') {
				j -= 1;
				i += 1;
				continue;
			}
		}
		if (i + 2 < str_len) {
			if (str[i] == '=' && str[i + 1] == '\r' && str[i + 2] == '\n') {
				j -= 1;
				i += 2;
				continue;
			}
		}
		found = str_chr(str_bits, str[i]);
		if (str_bits[found])
			break;
		str[j] = tolower(str[i]);
	}
	str[j] = '\0';
	*strend = str + j + 1;
	return str;
}

/*
 * Returns:
 * -1 on error
 *  0 if domain wasn't cached
 *  1 if domain was cached, and not blacklisted
 *  2 if domain was cached, and blacklisted.
 *
 * text != NULL: host blacklisted, text == reason.
 */
static int
cachefunc(char *uri, size_t urilen, char **text, int flag)
{
	static char     inbuf[2048];
	static stralloc cachefile = { 0 }, reason = { 0 };
	int             fd, i, n, textlen, match;
	struct stat     st;
	substdio        ss;

	if (!do_cache)
		return (0);
	if (uri[i = str_chr(uri, '/')]) {
		errno = EINVAL;
		return (-1);
	}
	if (!stralloc_copyb(&cachefile, "control/cache", 13))
		die_nomem();
	if (!stralloc_0(&cachefile))
		die_nomem();
	if (access(cachefile.s, F_OK))
		return (0);
	cachefile.len--;
	if (!stralloc_append(&cachefile, "/"))
		die_nomem();
	if (!stralloc_cats(&cachefile, uri))
		die_nomem();
	if (!stralloc_0(&cachefile))
		die_nomem();
	if (flag) { /*- add the cache */
		if (!access(cachefile.s, F_OK))
			return (0);
		if ((fd = open(cachefile.s, O_CREAT|O_WRONLY, *text ? 0600 : 0644)) == -1)
			my_error(cachefile.s, 0, 2);
		if (*text) {
			textlen = str_len(*text);
			if ((n = write(fd, *text, textlen)) == -1) {
				close(fd);
				my_error("write", 0, 1);
			}
		}
		if (close(fd))
			my_error(cachefile.s, 0, 1);
	} else {
		if (stat(cachefile.s, &st) == -1) {
			if (errno == ENOENT)
				return (0);
			my_error("stat", 0, 1);
			return -1;
		}
		if (time(0) > st.st_mtime + cachelifetime) {
			if (unlink(cachefile.s)) {
				my_error("unlink", 0, 1);
				return -1;
			}
			return (0);
		}
		if ((fd = open(cachefile.s, O_RDONLY)) == -1)
			my_error(cachefile.s, 0, 2);
		substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ss, &reason, &match, '\n') == -1) {
			close(fd);
			return -1;
		}
		*text = reason.s;
		close(fd);
		return (((st.st_mode & 07777) == 0600) ? 2 : 1);
	}
	return (0);
}

static int
getdnsip(stralloc *ip, stralloc *domain, int *code)
{
	char            x[IPFMT];
	ipalloc         ia = { 0 };
	int             len;

	if (!stralloc_copys(ip, ""))
		die_nomem();
	switch(dns_ip(&ia, domain))
	{
	case DNS_MEM:
		die_nomem();
	case DNS_SOFT:
		die_soft();
	case DNS_HARD:
		return 0;
	case 1:
		if (ia.len <= 0)
			die_soft();
	}
	if (code)
		*code = *(&ia.ix->ip.d[3]);
	len = ip_fmt(x, &ia.ix->ip);
	if (!stralloc_copyb(ip, x, len))
		die_nomem();
	return 0;
}

/*- SURBL: Check surbl rcpt whitelist.  */
int
srwcheck(char *arg, int len)
{
	int             j;

	if (!srwok)
		return 0;
	if (constmap(&mapsrw, arg, len))
		return 1;
	if ((j = byte_rchr(arg, len, '@')) < (len - 1)) {
		if (constmap(&mapsrw, arg + j, len - j))
			return 1;
	}
	return 0;
}

int
l2check(char *arg, int len)
{
	if (!l2ok)
		return (0);
	if (constmap(&mapl2, arg, len))
		return 1;
	return (0);
}

int
l3check(char *arg, int len)
{
	if (!l3ok)
		return (0);
	if (constmap(&mapl3, arg, len))
		return 1;
	return (0);
}

/*
 * Returns -1 on error.
 * Returns 0 if host does not exist.
 * Returns 1 if host exists.
 */
static int
checkwhitelist(char *hostname, int hostlen)
{
	int             len;
	char           *ptr;

	for (ptr = whitelist.s, len = 0;len < whitelist.len;) {
		if (!str_diffn(hostname, ptr, hostlen))
			return (1);
		len += (str_len(ptr) + 1);
		ptr = whitelist.s + len;
	}
	return (0);
}

static int
getreason(int code, char **text)
{
	static stralloc reason = { 0 };

	if (!stralloc_copyb(&reason, "blacklisted by ", 15))
		die_nomem();
	if (code & 64 && !stralloc_cats(&reason, debug ? "prolocation/jwspamspy" : "[jp]"))
		die_nomem();
	if (code & 32 && !stralloc_cats(&reason, debug ? "abusebutler " : "[ab]"))
		die_nomem();
	if (code & 16 && !stralloc_cats(&reason, debug ? "outblaze " : "[ob]"))
		die_nomem();
	if (code & 8 && !stralloc_cats(&reason, debug ? "phising " : "[ph]"))
		die_nomem();
	if (code & 2 && !stralloc_cats(&reason, debug ? "spamcop " : "[sc]"))
		die_nomem();
	if (code & 4 && !stralloc_cats(&reason, debug ? "w.stearns " : "[ws]"))
		die_nomem();
	if (!stralloc_0(&reason))
		die_nomem();
	*text = reason.s;
	return (code >= 2);
}

static int
checksurbl(char *uri, int urilen, char *surbldomain, char **text)
{
	static stralloc ip = { 0 };
	static stralloc host = { 0 };
	int             i, code = 0;

	if ((i = checkwhitelist(uri, urilen)) == -1)
		return -1;
	else
	if (i)
		return (0);
	if (stralloc_copys(&host, uri) == 0)
		die_nomem();
	if (stralloc_append(&host, ".") == 0)
		die_nomem();
	if (stralloc_cats(&host, surbldomain) == 0)
		die_nomem();
	if (!stralloc_0(&host))
		die_nomem();
	if (getdnsip(&ip, &host, &code) == -1)
		return -1;
	if (do_text && ip.len > 0) {
		if (text) {
			if ((*text = dns_text(host.s)))
				return 2;
		}
		return 1;
	}
	if (code > 1)
		return (getreason(code, text) ? 2 : 0);
	return 0;
}

static int
num_domains(const char *s)
{
	int             r = *s ? 1 : 0;

	while (*s) {
		if (*s++ == '.')
			++r;
	}
	return r;
}

static char *
remove_subdomains(char *orig, int output_domains)
{
	char           *s = orig + str_len((char *) orig);
	int             dots = 0;

	while (s > orig) {
		if (*s == '.')
			++dots;
		if (dots == output_domains) {
			++s;
			break;
		}
		--s;
	}
	return s;
}

/*
 * Returns 0 if URI was erronous.
 *         1 if URI was not blacklisted.
 *         2 if URI was blacklisted.
 */
static int
checkuri(char **ouri, char **text, size_t textlen)
{
	char           *uri = *ouri, *uriend, *ptr;
	char            ipuri[IPFMT];
	size_t          urilen = 0;
	ip_addr         ip;
	int             cached, blacklisted, i, level;

	if (case_diffb(uri, 4, "http"))
		return 0;
	uri += 4;

	/*- Check and skip http[s]?:[/\\][/\\]?  */
	if (*uri == 's')
		uri++;
	if (*uri == ':' && (uri[1] == '/' || uri[1] == '\\'))
		uri += 2;
	else 
		return 0;
	if (*uri == '/' || *uri == '\\')
		uri++;
	if (!isalpha(*uri) && !isdigit(*uri))
		return 0;
	uri_decode(uri, textlen, &uriend);
	*ouri = uriend;
	print_debug("Full    URI: ", uri, 0);
	uri[(urilen = str_cspn(uri, "/\\?"))] = '\0';
	if (uri[i = str_chr(uri, '@')])
		uri += (i + 1);
	uri[i = str_chr(uri, ':')] = 0;
	if (ip_scan(uri, &ip)) {
		ip_fmt(ipuri, &ip);
		uri = ipuri;
		print_debug("Proper IP: ", uri, 0);
	} else {
		urilen = str_len(uri);
		print_debug("Full domain: ", uri, 0);
		level = num_domains(uri);
		if (level > 2) {
			ptr = remove_subdomains(uri, 3);
			if (l3check(ptr, str_len(ptr)))
				uri = remove_subdomains(uri, 4);
			else {
				ptr = remove_subdomains(uri, 2);
				if (l2check(ptr, str_len(ptr)))
					uri = remove_subdomains(uri, 3);
				else
					uri = remove_subdomains(uri, 2);
			}
		} else
		if (level > 1) {
			ptr = remove_subdomains(uri, 2);
			if (l2check(ptr, str_len(ptr)))
				uri = remove_subdomains(uri, 3);
			else
				uri = remove_subdomains(uri, 2);
		}
		print_debug("       Part: ", uri, 0);
	}
	urilen = str_len(uri);
	cached = 1;
	blacklisted = 0;
	switch (cachefunc(uri, urilen, text, 0))
	{
	case 0:
		cached = 0;
		break;
	case 1:
		blacklisted = 0;
		break;
	case 2:
		blacklisted = 1;
		break;
	}
	if (cached == 0) {
		switch (checksurbl(uri, urilen, surbldomain.s, text))
		{
		case -1:
			return -1;
		case 0:
			blacklisted = 0;
			*text = (char *) 0;
			print_debug(uri, ": not blacklisted", 0);
			break;
		case 1:
			*text = "No reason given";
			blacklisted = 1;
			print_debug(uri, ": blacklisted. reason - ", *text);
			break;
		case 2:
			blacklisted = 2;
			print_debug(uri, ": blacklisted. reason - ", *text);
			break;
		}
		cachefunc(uri, urilen, text, 1);
	}
	return (blacklisted);
}

#define DEF_SURBL_DOMAIN "multi.surbl.org"

static int      do_surbl = 1;

static void
setup()
{
	char           *x, *y, *rcpt;
	int             i;

	if ((rcpt = env_get("QMAILRCPTS"))) {
		if ((srwok = control_readfile(&srw, "control/surblrcpt", 0)) == -1)
			die_control();
		if (srwok && !constmap_init(&mapsrw, srw.s, srw.len, 0))
			die_nomem();
	}
	for (x = y = rcpt, i = 0;rcpt && *x;x++, i++) {
		if (*x == '\n') {
			*x = 0;
			if (srwcheck(y, i)) {
				do_surbl = 0;
				return;
			}
			y = x + 1;
			*x = '\n';
			i = 0;
		}
	}
	if ((l2ok = control_readfile(&l2, "control/level2-tlds", 0)) == -1)
		die_control();
	if (l2ok && !constmap_init(&mapl2, l2.s, l2.len, 0))
		die_nomem();
	if ((l3ok = control_readfile(&l3, "control/level3-tlds", 0)) == -1)
		die_control();
	if (l3ok && !constmap_init(&mapl3, l3.s, l3.len, 0))
		die_nomem();
	switch (control_readline(&surbldomain, "control/surbldomain"))
	{
	case -1:
		die_control();
	case 0:
		if (!stralloc_copys(&surbldomain, DEF_SURBL_DOMAIN))
			die_nomem();
		/*- flow through */
	case 1:
		if (!stralloc_0(&surbldomain))
			die_nomem();
	}
	if ((x = env_get("CACHELIFETIME")))
		scan_int(x, &cachelifetime);
	else
	if (control_readint(&cachelifetime, "control/cachelifetime") == -1)
		die_control();
	if (control_readfile(&whitelist, "control/surbldomainwhite", 0) == -1)
		die_control();
	return;
}

int
main(int argc, char **argv)
{
	stralloc        base64out = { 0 }, boundary = { 0 }, uri = { 0 };
	stralloc       *ptr;
	char           *x, *reason = 0;
	int             opt, in_header = 1, i, total_bl = 0, blacklisted, match, html_plain_text,
					base64_decode, found_content_type = 0;

	if (!(x = env_get("SURBL")))
		do_surbl = 0;
	while ((opt = getopt(argc, argv, "vtc")) != opteof) {
		switch (opt) {
		case 'c':
			do_cache = 0;
			break;
		case 'v':
			debug = 1;
			break;
		case 't':
			do_text = 1;
			break;
		}
	}
	if (chdir(auto_qmail) == -1)
		die_control();
	if (do_surbl)
		setup();
	for (html_plain_text = base64_decode = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("getln: ", 0, 1);
		if (!match && line.len == 0)
			break;
		if (substdio_put(&ssout, line.s, line.len))
			die_write();
		if (!do_surbl)
			continue;
		if (in_header) {
			if (!str_diffn(line.s, "Content-Type: ", 14)) {
				found_content_type = 1;
			}
			if (found_content_type) {
				for (i = 0;i < line.len; i++) {
					if (case_startb(line.s + i, line.len - i, "boundary=")) {
						if (line.s[i + 9] == '\"' && line.s[line.len -2] == '\"')
						{
							if (!stralloc_copyb(&boundary, line.s + i + 10, line.len -i - 12))
								die_nomem();
						} else
						if (!stralloc_copyb(&boundary, line.s + i + 9, line.len - i - 10))
							die_nomem();
						if (!stralloc_0(&boundary))
							die_nomem();
						boundary.len--;
					}
				}
			}
			if (!mess822_ok(&line))
				in_header = 0;
		} else {
			if (!str_diffn(line.s, "Content-Type: ", 14)) {
				if (!str_diffn(line.s + 14, "message/rfc822", 14) ||
					!str_diffn(line.s + 14, "text/html", 9) ||
					!str_diffn(line.s + 14, "text/plain", 10))
						html_plain_text = 1;
				else
						html_plain_text = 0;
			}
			if (html_plain_text && !str_diffn(line.s, "Content-Transfer-Encoding: ", 27)) {
				if (!str_diffn(line.s + 27, "base64", 6))
					base64_decode = 1;
				else
					base64_decode = 0;
			}
			if (line.len == 1)
				continue;
			if (base64_decode) {
				if (!str_diffn(line.s, "Content-", 8))
					continue;
				if (!str_diffn(line.s + 2, boundary.s, boundary.len)) {
					base64_decode = 0;
					continue;
				}
				if (b64decode((const unsigned char *) line.s, line.len - 1, &base64out) == -1)
					die_nomem();
				ptr = &base64out;
			} else
				ptr = &line;
			for (blacklisted = -1, i = 0;i < ptr->len; i++) {
				if (case_startb(line.s + i, ptr->len - i, "http:") ||
						case_startb(line.s + i, ptr->len - i, "https:")) {
					x = ptr->s + i;
					if (!stralloc_copyb(&uri, ptr->s + i, ptr->len - i) || !stralloc_0(&uri))
						die_nomem();
					switch (checkuri(&x, &reason, ptr->len - i))
					{
					case -1:
						my_error("checkuri", 0, 111);
					case 0: /*- no valid uri in line */
						blacklisted = 0;
						break;
					case 1:
					case 2:
						blacklisted = 1;
						break;
					}
				}
				if (blacklisted == 1) {
					total_bl++;
					break;
				}
			}
		}
	} /*- for (html_plain_text = base64_decode = 0;;) { */
	if (substdio_flush(&ssout) == -1)
		die_write();
	if (do_surbl && total_bl) {
		logerr("Dmessage contains an URL listed in SURBL blocklist [");
		i = str_chr(uri.s, '\n');
		if (uri.s[i])
			uri.s[i] = '\0';
		logerr(uri.s);
		logerrf("]");
		_exit (88); /*- custom error */
	}
	return (0);
}

void
getversion_surblfilter_c()
{
	static char    *x = "$Id: surblfilter.c,v 1.4 2011-07-13 22:28:32+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

