/*
 * $Id: qmail-dkim.c,v 1.76 2024-01-10 23:01:23+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sgetopt.h"
#include "substdio.h"
#include "open.h"
#include "qmail.h"
#include "sig.h"
#include "scan.h"
#include "case.h"
#include "fmt.h"
#include "fd.h"
#include "alloc.h"
#include "str.h"
#include "stralloc.h"
#include "datetime.h"
#include "now.h"
#include "wait.h"
#include "auto_qmail.h"
#include "env.h"
#include "control.h"
#include "error.h"
#include "getDomainToken.h"
#include "makeargs.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "dkim.h"
#include <openssl/evp.h>

#define DKIM_MALLOC(n)     OPENSSL_malloc(n)
#define DKIM_MFREE(s)      OPENSSL_free(s); s = NULL;
#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */
#define ADDR 1003
#ifndef HAVE_EVP_SHA256
#define HAVE_EVP_SHA256
#endif
#define strncasecmp(x,y,z) case_diffb((x), (z), (y))
#define strcasecmp(x,y)    case_diffs((x), (y))

extern char    *dns_text(char *);

char            inbuf[4096];
char            outbuf[256];
struct substdio ssin;
struct substdio ssout;

datetime_sec    starttime;
struct datetime dt;
unsigned long   mypid;
unsigned long   uid;
char           *pidfn;
int             messfd;
int             readfd;
DKIMContext     ctxt;
static char    *dkimsignoptions;
static char    *binqqargs[2] = { "bin/qmail-queue", 0 };
static char    *controldir;
static stralloc fntmp = {0};
static stralloc bouncehost = {0};

void
die(int e, int what)
{
	if (!what)
		_exit(e);
	(what == 1 ? DKIMSignFree : DKIMVerifyFree) (&ctxt);
	_exit(e);
}

void
die_write()
{
	die(53, 0);
}

void
die_read()
{
	die(54, 0);
}

void
sigalrm()
{
	/*- thou shalt not clean up here */
	die(52, 0);
}

void
sigbug()
{
	die(81, 0);
}

int DKIM_CALL
SignThisHeader(const char *szHeader)
{
	char           *excl = 0, *cptr, *ptr;
	int             i;
	if ((!strncasecmp((char *) szHeader, "X-", 2) && strncasecmp((char *) szHeader, "X-Mailer:", 9))
			|| !strncasecmp((char *) szHeader, "Received:", 9)
			|| !strncasecmp((char *) szHeader, "Authentication-Results:", 23)
			|| !strncasecmp((char *) szHeader, "Arc-Authentication-Results:", 27)
			|| !strncasecmp(szHeader, "DKIM-Signature:", 15)
			|| !strncasecmp(szHeader, "DomainKey-Signature:", 20)
			|| !strncasecmp((char *) szHeader, "Return-Path:", 12))
		return 0;
	if (!(excl = env_get("EXCLUDE_DKIMSIGN")))
		return 1;
	for (i = 0, cptr = ptr = excl; *ptr; ptr++) {
		if (*ptr == ':') {
			if (strncasecmp((char *) szHeader, cptr, i) == 0)
				return 0;
			cptr = ptr + 1;
			i = 0;
		} else
			i++;
	}
	if (strncasecmp((char *) szHeader, cptr, i) == 0)
		return 0;
	return 1;
}

void
custom_error(char *program, char *type, char *message, char *extra, char *code)
{
	char           *c;
	char            errbuf[256];
	int             errfd = -1;
	struct substdio sserr;

	if (errfd == -1) {
		if (!(c = env_get("ERROR_FD")))
			errfd = CUSTOM_ERR_FD;
		else
			scan_int(c, &errfd);
	}
	substdio_fdbuf(&sserr, write, errfd, errbuf, sizeof(errbuf));
	if (substdio_put(&sserr, type, 1) == -1 ||
			substdio_puts(&sserr, program) == -1 ||
			substdio_put(&sserr, ": ", 2) ||
			substdio_puts(&sserr, message) == -1)
		_exit(53);
	if (extra && (substdio_put(&sserr, ": ", 2) == -1 || substdio_puts(&sserr, extra) == -1))
		_exit(53);
	if (code) {
		if (substdio_put(&sserr, " (#", 3) == -1)
			_exit(53);
		c = (*type == 'D') ? "5" : "4";
		if (substdio_put(&sserr, c, 1) == -1 ||
				substdio_put(&sserr, code + 1, 4) == -1 ||
				substdio_put(&sserr, ")", 1) == -1)
			_exit(53);
	}
	if (substdio_flush(&sserr) == -1)
		_exit(53);
	_exit(88);
}

void
maybe_die_dkim(int e)
{
	switch (e)
	{
	case DKIM_OUT_OF_MEMORY:
	case DKIM_BUFFER_TOO_SMALL:
		_exit (51);
	case DKIM_INVALID_CONTEXT:
		custom_error("qmail-dkim", "Z", "DKIMContext structure invalid for this operation", 0, "X.3.0");
	case DKIM_NO_SENDER:
		custom_error("qmail-dkim", "Z", "Could not find From: or Sender: header in message", 0, "X.1.7");
	case DKIM_BAD_PRIVATE_KEY:
		custom_error("qmail-dkim", "D", "Could not parse private key", 0, "X.7.5");
	default:
		return;
	}
}

unsigned int
pidfmt(char *s, unsigned long seq)
{
	unsigned int    i;
	unsigned int    len;
	char           *tmpdir;

	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	len = 0;
	i = fmt_str(s, tmpdir);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "/qmail-dkim.");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, starttime);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, seq);
	len += i;
	if (s)
		s += i;
	++len;
	if (s)
		*s++ = 0;

	return len;
}

void
pidopen()
{
	unsigned int    len;
	unsigned long   seq;

	seq = 1;
	len = pidfmt((char *) 0, seq);
	if (!(pidfn = alloc(len)))
		die(51, 0);
	for (seq = 1; seq < 10; ++seq) {
		if (pidfmt((char *) 0, seq) > len)
			die(81, 0); /*- paranoia */
		pidfmt(pidfn, seq);
		if ((messfd = open_excl(pidfn)) != -1)
			return;
	}
	die(63, 0);
}

static char    *dkimsign = 0;
static char    *dkimverify = 0;
static char    *dkimpractice =  "FGHIJKLMNPQRSTUVWX";
static stralloc dkimoutput = { 0 };  /*- DKIM-Signature */
static stralloc dksignature = { 0 }; /*- content of private signature */
static stralloc sigdomains = { 0 };  /*- domains which must have signatures */
static stralloc nsigdomains = { 0 }; /*- domains which do not have signatures */
static stralloc dkimopts = { 0 };
static stralloc dkimkeys = { 0 };
static char    *dkimqueue = 0;

char           *
replace_pct(char *keyfn, char *domain, int pos, int *replace)
{
	char           *p, *t, *s;
	int             i, d, r, len;
	static stralloc tmp = {0};

	if (!domain) {
		tmp.len = 0;
		for (p = keyfn, len = 0; *p; p++) {
			if (*p == '%') {
				if (tmp.len && *(p - 1) == '/' && *(p + 1) == '/') /*- replace // with single / */
					tmp.len--;
			} else
			if (!stralloc_append(&tmp, p))
				die(51, 1);
		}
		if (!stralloc_0(&tmp))
			die(51, 1);
		return tmp.s;
	}
	if (!keyfn[pos + 1]) { /*- file has % as the last component (implies selector is %) */
		len = pos + (d = fmt_str(0, domain));
		r = 0;
		if (replace)
			*replace = 1;
	} else
		len = pos + (d = fmt_str(0, domain)) + (r = fmt_str(0, keyfn + pos + 1));
	if (!(t = (char *) alloc((len + 1) * sizeof(char))))
		die(51, 1);
	s = t;
	s += fmt_strn(t, keyfn, pos);
	s += fmt_strn(t + pos, domain, d);
	if (keyfn[pos + 1])
		s += fmt_strn(t + pos + d, keyfn + pos + 1, r);
	*s = 0;
	i = str_rchr(t, '%');
	if (t[i]) {
		p = replace_pct(t, domain, i, replace);
		alloc_free(t);
		return p;
	} else {
		if (!stralloc_copyb(&tmp, t, len + 1))
			die(51, 1);
		alloc_free(t);
		return tmp.s;
	}
}

int
dkim_setoptions(DKIMSignOptions *opts, char *signOptions)
{
	int             ch, argc;
	char          **argv;

	sgoptind = 1;
	sgoptpos = 0;
	opts->nCanon = DKIM_SIGN_RELAXED;					/*- c */
	opts->nIncludeBodyLengthTag = 0;					/*- l */
	opts->nIncludeQueryMethod = 0;						/*- q */
	opts->nIncludeTimeStamp = 0;						/*- t */
	opts->nIncludeCopiedHeaders = 0;					/*- h */
	opts->szIdentity[0] = '\0';
	opts->expireTime = starttime + 604800;	// expires in 1 week
	opts->nHash = DKIM_HASH_SHA256;
	str_copy(opts->szRequiredHeaders, "NonExistent");
	if (!signOptions)
		return (0);
	if (!stralloc_copys(&dkimopts, "dkim ") ||
			!stralloc_cats(&dkimopts, signOptions) ||
			!stralloc_0(&dkimopts))
		die(51, 0);
	if (!(argv = makeargs(dkimopts.s)))
		die(51, 0);
	for (argc = 0;argv[argc];argc++);
	while ((ch = sgopt(argc, argv, "b:c:li:qthx:z:")) != sgoptdone) {
		switch (ch)
		{
		case 'b':
			break;
		case 'c':
			switch (*optarg)
			{
			case 'r':
				opts->nCanon = DKIM_SIGN_RELAXED;
				break;
			case 's':
				opts->nCanon = DKIM_SIGN_SIMPLE;
				break;
			case 't':
				opts->nCanon = DKIM_SIGN_RELAXED_SIMPLE;
				break;
			case 'u':
				opts->nCanon = DKIM_SIGN_SIMPLE_RELAXED;
				break;
			default:
				free_makeargs(argv);
				return (1);
			}
			break;
		case 'l': /*- body length tag */
			opts->nIncludeBodyLengthTag = 1;
			break;
		case 'q': /*- query method tag */
			opts->nIncludeQueryMethod = 1;
			break;
		case 't': /*- timestamp tag */
			opts->nIncludeTimeStamp = 1;
			break;
		case 'h':
			opts->nIncludeCopiedHeaders = 1;
			break;
		case 'i':	/*- identity */
			if (*optarg == '-') /* do not use i= tag */
				opts->szIdentity[0] = '\0';
			else
				str_copyb(opts->szIdentity, optarg, sizeof(opts->szIdentity) - 1);
			break;
		case 'x': /*- expire time */
			if (*optarg == '-')
				opts->expireTime = 0;
			else
				opts->expireTime = starttime + atoi(optarg);
			break;
		case 'z': /*- sign rsa-sha1, rsa-sha256, rsa-sha1+rsa-sha256 or ed25519 */
			switch (*optarg)
			{
			case '1':
				opts->nHash = DKIM_HASH_SHA1;
				break;
#ifdef HAVE_EVP_SHA256
			case '2':
				opts->nHash = DKIM_HASH_SHA256;
				break;
			case '3':
				opts->nHash = DKIM_HASH_SHA1_AND_SHA256;
				break;
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
			case '4':
				opts->nHash = DKIM_HASH_ED25519;
				break;
#endif
			default:
				free_makeargs(argv);
				return (1);
			}
			break;
		default:
			free_makeargs(argv);
			return (1);
		} /*- switch (ch) */
	} /*- while (1) */
	free_makeargs(argv);
	return (0);
}

void
format_controlfile(char *fn, int what)
{
	if (!stralloc_copys(&fntmp, controldir) ||
			!stralloc_append(&fntmp, "/") ||
			!stralloc_cats(&fntmp, fn) ||
			!stralloc_0(&fntmp))
		die(51, what);
}

static void
write_signature(char *domain, DKIMSignOptions *opts, size_t selector_size)
{
	char           *pSig, *keyfn, *ptr, *selector;
	int             i, r_selector, pct_found;
	static stralloc tmp = { 0 };

	if (env_get("NODKIMKEYS")) {
		i = 0;
		keyfn = dkimsign;
	} else {
		format_controlfile("dkimkeys", 1);
		if ((i = control_readfile(&dkimkeys, fntmp.s, 0)) == -1)
			custom_error("qmail-dkim", "Z", "unable to read dkimkeys.", 0, "X.3.0");
		else
		if (!i || !(keyfn = getDomainToken(domain, &dkimkeys))) {
			i = 0;
			keyfn = dkimsign;
		} else {
			ptr = env_get("DKIMSIGN");
			/*- don't allow DKIMSIGN to be altered by last field in dkimkeys */
			if (str_diff(ptr, keyfn) && !env_put2("DKIMSIGN", keyfn))
				die(51, 1);
			ptr = env_get("DKIMSIGNOPTIONS");
			if (!dkimsignoptions || str_diff(dkimsignoptions, ptr)) {
				dkimsignoptions = ptr;
				if (dkim_setoptions(opts, dkimsignoptions))
					custom_error("qmail-dkim", "Z", "Invalid DKIMSIGNOPTIONS", 0, "X.3.0");
				DKIMSignReplaceHash(&ctxt, opts);
			}
		}
	}
	if (keyfn[0] != '/') {
		if (!stralloc_copys(&tmp, controldir) ||
				!stralloc_append(&tmp, "/") ||
				!stralloc_cats(&tmp, keyfn))
			die(51, 1);
	} else
	if (!stralloc_copys(&tmp, keyfn))
		die(51, 1);
	if (!stralloc_0(&tmp))
		die(51, 1);
	/*
	 * it is possible that selector was chosen based
	 * on DKIMSIGN.
	 * If the selector is % or keyfn has been overriden
	 * by dkimkeys control file, we need to change
	 * the selector
	 */
	r_selector = i ? 1 : 0; /*- for replacing selector later */
	i = str_rchr(tmp.s, '%');
	if (tmp.s[i]) {
		pct_found = 1;
		keyfn = replace_pct(tmp.s, domain, i, &r_selector);
		if (access(keyfn, F_OK)) {
			if (errno != error_noent && errno != error_isdir)
				custom_error("qmail-dkim", "Z", "unable to read private key.", 0, "X.3.0");
			keyfn = replace_pct(tmp.s, 0, 0, 0);
		}
	} else {
		pct_found = 0;
		keyfn = tmp.s;
	}
	switch (control_readnativefile(&dksignature, keyfn, 1))
	{
	case 0: /*- missing signature file */
		DKIMSignFree(&ctxt);
		/*
		 * You may have multiple domains, but may chose to sign
		 * only for few domains which have the key present. Do not
		 * treat domains with missing key as an error.
		 */
		if (pct_found)
			return;
		die(32, 0);
	case 1:
		break;
	default:
		DKIMSignFree(&ctxt);
		if (errno == error_isdir && pct_found)
			return;
		custom_error("qmail-dkim", "Z", "unable to read private key.", 0, "X.3.0");
	}
	for (i = 0; i < dksignature.len; i++) {
		if (dksignature.s[i] == '\0')
			dksignature.s[i] = '\n';
	}
	if (!stralloc_0(&dksignature))
		die(51, 1);
	if (r_selector) { /*- replace selector */
		selector = ptr = keyfn;
		while (*ptr) {
			if (*ptr == '/' && *(ptr + 1))
				selector = ptr + 1;
			ptr++;
		}
		str_copyb(opts->szSelector, selector, selector_size);
		DKIMSignReplaceSelector(&ctxt, opts);
	}
	i = DKIMSignGetSig2(&ctxt, dksignature.s, &pSig);
	maybe_die_dkim(i);
	if (pSig) {
		if (!stralloc_catb(&dkimoutput, pSig, str_len(pSig)) ||
				!stralloc_cats(&dkimoutput, "\n"))
			die(51, 1);
	}
	DKIMSignFree(&ctxt);
}

int
ParseTagValues(char *list, char *letters[], char *values[])
{
	char           *t, *ptr, *key;
	int             i;

	/*- start with all args unset */
	for (i = 0; letters[i]; i++)
		values[i] = 0;
	key = 0;
	for(ptr = list;*ptr;) {
		if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\r') || (*ptr == '\n')) /*- FWS */
			*ptr++ = 0;
		if (!key)
			key = ptr;
		if (*ptr == '=') {
			*ptr = 0;
			for (i = 0;letters[i];i++) {
				if (!str_diff(letters[i], key)) {
					ptr++;
					for (;*ptr;) {
						if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\r') || (*ptr == '\n')) {
							ptr++;
							continue;
						}
						break;
					}
					values[i] = ptr;
					for(;*ptr && *ptr != ';';ptr++);
					t = ptr;
					if (*ptr)
						*ptr++ = 0;
					for(;t != values[i];t--) { /*- RFC 4871 3.2 */
						if ((*t == ' ') || (*t == '\t') || (*t == '\r') || (*t == '\n')) {
							*t = 0;
							continue;
						}
						break;
					}
					key = 0;
					break;
				}
			}
		} else
			ptr++;
	}
	return (0);
}

int
checkSSP(char *domain, int *bTesting)
{
	char           *query, *results;
	char           *tags[] = { "dkim", "t", 0};
	char           *values[2];
	int             bIsParentSSP = 0, iSSP = DKIM_SSP_UNKNOWN;

	*bTesting = 0;
	if (!(query = DKIM_MALLOC(str_len("_ssp._domainkey.") + str_len(domain) + 1)))
		die(51, 0);
	sprintf(query, "_ssp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!str_diff(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_SSP_TEMPFAIL;
	} else
	if (!str_diff(results, "e=perm;")) {
		results = dns_text(domain);
		if (!str_diff(results, "e=temp;")) {
			DKIM_MFREE(results);
			return DKIM_SSP_TEMPFAIL;
		} else
		if (!str_diff(results, "e=perm;")) {
			DKIM_MFREE(results);
			return DKIM_SSP_SCOPE;
		}
		bIsParentSSP = 1;
	}
	/*-
	 * PG.1 2013-01-03
	 * Bug fix by Piotr Gronek, Faculty of Physics & Applied Computer Science, Poland 2013-01-03
	 * Deallocating storage for 'results' here is premature - moved beyond last reference to it.
	 *
	 */
	if (!ParseTagValues(results, tags, values)) {
		DKIM_MFREE(results);
		return DKIM_SSP_UNKNOWN;
	}
	DKIM_MFREE(results);
	if (values[0] != NULL) {
		if (strcasecmp(values[0], "all") == 0)
			iSSP = DKIM_SSP_ALL;
		else
		if (strcasecmp(values[0], "strict") == 0)
			iSSP = DKIM_SSP_STRICT;
	}
	// flags
	if (values[1] != NULL) {
		char           *s, *p;
		for (p = values[1], s = values[1]; *p; p++) {
			if (*p == '|')
				*p = 0;
			else
				continue;
			if (!str_diff(s, "y"))
				*bTesting = 1;
			else
			if (!str_diff(s, "s")) {
				if (bIsParentSSP) {
					/*-
					 * this is a parent's SSP record that should not apply to subdomains
					 * the message is non-suspicious
					 */
					*bTesting = 0;
					return (DKIM_SSP_UNKNOWN);
				}
			}
			s = p + 1;
		}
	}
	return iSSP;
}

int
checkADSP(char *domain)
{
	char           *query, *results;
	char           *tags[] = { "dkim", 0};
	char           *values[1];

	results = dns_text(domain);
	if (!str_diff(results, "e=perm;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	} else
	if (!str_diff(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	if (!(query = DKIM_MALLOC(str_len("_adsp._domainkey.") + str_len(domain) + 1))) {
		DKIM_MFREE(results);
		die(51, 0);
	}
	sprintf(query, "_adsp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!str_diff(results, "e=perm;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	} else
	if (!str_diff(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	/*-
	 * PG.1 2013-01-03
	 * Bug fix by Piotr Gronek, Faculty of Physics & Applied Computer Science, Poland 2013-01-03
	 *
	 * Deallocating storage for 'results' here is premature - moved beyond last reference to it.
	 *
	 */
	if (!ParseTagValues(results, tags, values)) {
		DKIM_MFREE(results);
		return DKIM_SSP_UNKNOWN;
	}
	DKIM_MFREE(results);
	if (values[0] != NULL) {
		if (strcasecmp(values[0], "all") == 0)
			return (DKIM_ADSP_ALL);
		else
		if (strcasecmp(values[0], "discardable") == 0)
			return (DKIM_ADSP_DISCARDABLE);
	}
	return DKIM_ADSP_UNKNOWN; /*- No ADSP Record */
}

void
dkimverify_exit(int dkimRet, char *status, char *code)
{
	if (dkimRet < 0) {
		if (dkimverify[str_chr(dkimverify, 'F' - dkimRet)])
			custom_error("qmail-dkim", "D", status, 0, code);
		if (dkimverify[str_chr(dkimverify, 'f' - dkimRet)])
			custom_error("qmail-dkim", "Z", status, 0, code);
	} else {
		if (dkimverify[str_chr(dkimverify, 'A' + dkimRet)])
			custom_error("qmail-dkim", "D", status, 0, code);
		if (dkimverify[str_chr(dkimverify, 'a' + dkimRet)])
			custom_error("qmail-dkim", "Z", status, 0, code);
	}
}

void
writeHeaderNexit(int ret, int origRet, int resDKIMSSP, int resDKIMADSP, int useSSP, int useADSP)
{
	char           *dkimStatus = 0, *sspStatus = 0, *adspStatus = 0, *code = 0, *orig = 0;
	char            strnum[FMT_ULONG];

	switch (ret)
	{
	case DKIM_SUCCESS:			/*- 0 */ /*- A */
		dkimStatus = "good        ";
		code = "X.7.0";
		break;
	case DKIM_FINISHED_BODY:	/*- 1 process result: no more message body is needed */
		dkimStatus = "process result: no more message body is needed";
		code = "X.7.0";
		break;
	case DKIM_PARTIAL_SUCCESS:	/*- 2 verify result: at least one but not all signatures verified */
		dkimStatus = "verify result: at least none but not all signatures verified";
		code = "X.7.0";
		break;
	case DKIM_NEUTRAL:			/*- 3 verify result: no signatures verified but message is not suspicious */
		dkimStatus = "verify result: no signatures verified but message is not suspicious";
		code = "X.7.0";
		break;
	case DKIM_SUCCESS_BUT_EXTRA:/*- 4 signature result: signature verified but it did not include all of the body */
		dkimStatus = "signature result: signature verified but it did not include all of the body";
		code = "X.7.0";
		break;
	case DKIM_FAIL:				/*- -1 */ /*- F */
		dkimStatus = "DKIM Signature verification failed";
		code = "X.7.0";
		break;
	case DKIM_BAD_SYNTAX:		/*- -2 */ /*- G */
		dkimStatus = "signature error: DKIM-Signature could not parse or has bad tags/values";
		code = "X.7.5";
		break;
	case DKIM_SIGNATURE_BAD:	/*- -3 */
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
		dkimStatus = "signature error: RSA/ED25519 verify failed";
#else
		dkimStatus = "signature error: RSA verify failed";
#endif
		code = "X.7.5";
		break;
	case DKIM_SIGNATURE_BAD_BUT_TESTING:
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
		dkimStatus = "signature error: RSA/ED25519 verify failed but testing";
#else
		dkimStatus = "signature error: RSA verify failed but testing";
#endif
		code = "X.7.5";
		break;
	case DKIM_SIGNATURE_EXPIRED:
		dkimStatus = "signature error: x= is old";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_INVALID:
		dkimStatus = "signature error: selector doesn't parse or contains invalid values";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_GRANULARITY_MISMATCH:
		dkimStatus = "signature error: selector g= doesn't match i=";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_KEY_REVOKED:
		dkimStatus = "signature error: selector p= empty";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG:
		dkimStatus = "signature error: selector domain name too long to request";
		code = "X.7.0";
		break;
	case DKIM_SELECTOR_DNS_TEMP_FAILURE:
		dkimStatus = "signature error: temporary dns failure requesting selector";
		code = "X.7.0";
		break;
	case DKIM_SELECTOR_DNS_PERM_FAILURE:
		dkimStatus = "signature error: permanent dns failure requesting selector";
		code = "X.7.0";
		break;
	case DKIM_SELECTOR_PUBLIC_KEY_INVALID:
		dkimStatus = "signature error: selector p= value invalid or wrong format";
		code = "X.7.5";
		break;
	case DKIM_NO_SIGNATURES:
		dkimStatus = "no signatures";
		code = "X.7.5";
		break;
	case DKIM_NO_VALID_SIGNATURES:
		dkimStatus = "no valid signatures";
		code = "X.7.5";
		break;
	case DKIM_BODY_HASH_MISMATCH:
		dkimStatus = "signature verify error: message body does not hash to bh value";
		code = "X.7.7";
		break;
	case DKIM_SELECTOR_ALGORITHM_MISMATCH:
		dkimStatus = "signature error: selector h= doesn't match signature a=";
		code = "X.7.7";
		break;
	case DKIM_STAT_INCOMPAT:
		dkimStatus = "signature error: incompatible v=";
		code = "X.7.6";
		break;
	case DKIM_UNSIGNED_FROM:
		dkimStatus = "signature error: not all message's From headers in signature";
		code = "X.7.7";
		break;
	default:
		dkimStatus = "error";
		code = "X.3.0";
		break;
	}
	if (useSSP && resDKIMSSP != -1) {
		switch(resDKIMSSP)
		{
			case DKIM_SSP_ALL:
				sspStatus = (char *) "all;";
				break;
			case DKIM_SSP_STRICT:
				sspStatus = (char *) "strict;";
				break;
			case DKIM_SSP_SCOPE:
				sspStatus = (char *) "out of scope;";
				break;
			case DKIM_SSP_TEMPFAIL:
				sspStatus = (char *) "temporary failure;";
				break;
			case DKIM_SSP_UNKNOWN:
			default:
				sspStatus = (char *) "unknown;";
				break;
		}
	}
	if (useADSP && resDKIMADSP != -1) {
		switch(resDKIMADSP)
		{
			case DKIM_ADSP_ALL:
				adspStatus = (char *) "all;";
				break;
			case DKIM_ADSP_DISCARDABLE:
				adspStatus = (char *) "discardable;";
				break;
			case DKIM_ADSP_SCOPE:
				adspStatus = (char *) "out of scope;";
				break;
			case DKIM_ADSP_TEMPFAIL:
				adspStatus = (char *) "temporary failure;";
				break;
			case DKIM_ADSP_UNKNOWN:
			default:
				adspStatus = (char *) "unknown ;";
				break;
		}
	}
	if (!stralloc_copys(&dkimoutput, "DKIM-Status: ") ||
			!stralloc_cats(&dkimoutput, dkimStatus))
		die(51, 0);
	if (origRet != DKIM_MAX_ERROR && ret != origRet) {
		if (!stralloc_cats(&dkimoutput, "\n\t(old="))
			die(51, 0);
		switch (origRet)
		{
		case DKIM_SUCCESS:			/*- 0 */ /*- A */
			orig = "SUCCESS";
			break;
		case DKIM_FINISHED_BODY:	/*- 1 process result: no more message body is needed */
			orig = "FINISHED BODY";
			break;
		case DKIM_PARTIAL_SUCCESS:	/*- 2 verify result: at least one but not all signatures verified */
			orig = "PARTIAL SUCCESS";
			break;
		case DKIM_NEUTRAL:			/*- 3 verify result: no signatures verified but message is not suspicious */
			orig = "NEUTRAL";
			break;
		case DKIM_SUCCESS_BUT_EXTRA:/*- 4 signature result: signature verified but it did not include all of the body */
			orig = "SUCCESS(BUT EXTRA)";
			break;
		case DKIM_FAIL:				/*- -1 */ /*- F */
			orig = "FAIL";
			break;
		case DKIM_BAD_SYNTAX:		/*- -2 */ /*- G */
			orig = "BAD SYNTAX";
			break;
		case DKIM_SIGNATURE_BAD:	/*- -3 */
			orig = "SIGNATURE BAD";
			break;
		case DKIM_SIGNATURE_BAD_BUT_TESTING:
			orig = "SIGNATURE BAD (TESTING)";
			break;
		case DKIM_SIGNATURE_EXPIRED:
			orig = "SIGNATURE EXPIRED";
			break;
		case DKIM_SELECTOR_INVALID:
			orig = "SELECTOR INVALID";
			break;
		case DKIM_SELECTOR_GRANULARITY_MISMATCH:
			orig = "SELECTOR GRANULARITY MISMATCH";
			break;
		case DKIM_SELECTOR_KEY_REVOKED:
			orig = "SELECTOR KEY REVOKED";
			break;
		case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG:
			orig = "DOMAIN NAME TOO LONG";
			break;
		case DKIM_SELECTOR_DNS_TEMP_FAILURE:
			orig = "DNS TEMP FAILURE";
			break;
		case DKIM_SELECTOR_DNS_PERM_FAILURE:
			orig = "DNS PERM FAILURE";
			break;
		case DKIM_SELECTOR_PUBLIC_KEY_INVALID:
			orig = "PUBLIC KEY INVALID";
			break;
		case DKIM_NO_SIGNATURES:
			orig = "NO SIGNATURES";
			break;
		case DKIM_NO_VALID_SIGNATURES:
			orig = "NO VALID SIGNATURES";
			break;
		case DKIM_BODY_HASH_MISMATCH:
			orig = "BODY HASH MISMATCH";
			break;
		case DKIM_SELECTOR_ALGORITHM_MISMATCH:
			orig = "ALGORITHM MISMATCH";
			break;
		case DKIM_STAT_INCOMPAT:
			orig = "STAT INCOMPAT";
			break;
		case DKIM_UNSIGNED_FROM:
			orig = "UNSIGNED FROM";
			break;
		default:
			orig = "Unkown error";
			break;
		}
		if (!stralloc_cats(&dkimoutput, orig) ||
				!stralloc_cats(&dkimoutput, ":"))
			die(51, 0);
		if (origRet < 0) {
			if (!stralloc_cats(&dkimoutput, "-"))
				die(51, 0);
			strnum[fmt_ulong(strnum, 0 - origRet)] = 0;
		} else
			strnum[fmt_ulong(strnum, origRet)] = 0;
		if (!stralloc_cats(&dkimoutput, strnum) ||
				!stralloc_cats(&dkimoutput, ")"))
			die(51, 0);
	}
	if (!stralloc_cats(&dkimoutput, "\n"))
		die(51, 0);
	if (useSSP && sspStatus) {
		if (!stralloc_cats(&dkimoutput, "X-DKIM-SSP: ") ||
				!stralloc_cats(&dkimoutput, sspStatus) ||
				!stralloc_cats(&dkimoutput, "\n"))
			die(51, 0);
	}
	if (useADSP && adspStatus) {
		if (!stralloc_cats(&dkimoutput, "X-DKIM-ADSP: ") ||
				!stralloc_cats(&dkimoutput, adspStatus) ||
				!stralloc_cats(&dkimoutput, "\n"))
			die(51, 0);
	}
	dkimverify_exit(ret, dkimStatus, code);
	return;
}

int
checkPractice(int dkimRet, int useADSP, int useSSP)
{
	char           *ptr;

	if (!(ptr = env_get("DKIMPRACTICE"))) {
		/*- if SIGN_PRACTICE="local" then you can use DKIMVERIFY env variable too */
		if (!useADSP && !useSSP)
			dkimpractice = dkimverify; /*- DKIMVERIFY env variable */
		else
			return (0);
	} else
		dkimpractice = ptr;
	if (!*dkimpractice) {
		if (dkimRet < 0 || dkimRet == DKIM_3PS_SIGNATURE)
			return (1);
		return (0);
	}
	if (dkimRet < 0) {
		if (dkimpractice[str_chr(dkimpractice, 'F' - dkimRet)])
			return (1);
		if (dkimpractice[str_chr(dkimpractice, 'f' - dkimRet)])
			return (1);
	} else {
		if (dkimpractice[str_chr(dkimpractice, 'A' + dkimRet)])
			return (1);
		if (dkimpractice[str_chr(dkimpractice, 'a' + dkimRet)])
			return (1);
	}
	return (0);
}

static char    *callbackdata;

int
dns_bypass(const char *domain, char *buffer, int maxlen)
{
	if (callbackdata) {
		str_copy(buffer, callbackdata);
		return 0;
	} else
		return 1;
}

int
main(int argc, char *argv[])
{
	int             pim[2];
	int             wstat;
	int             resDKIMSSP = -1, resDKIMADSP = -1, useSSP = 0, useADSP = 0, accept3ps = 0;
	int             sCount = 0, sSize = 0;
	int             ret = 0, origRet = DKIM_MAX_ERROR, i, nSigCount = 0, len, token_len;
	unsigned long   pid;
	char           *selector = NULL, *ptr;
	stralloc        dkimfn = {0};
	DKIMVerifyDetails *pDetails;
	DKIMSignOptions   sopts = { 0 };
	DKIMVerifyOptions vopts = { 0 };

	starttime = now();
	sig_blocknone();
	umask(033);
	if (!(controldir = env_get("CONTROLDIR")))
		controldir = "control";
	if (chdir(auto_qmail) == -1)
		die(61, 0);
	dkimqueue = env_get("DKIMQUEUE");
	if (dkimqueue && *dkimqueue)
		binqqargs[0] = dkimqueue;
	dkimsign = env_get("DKIMSIGN");
	dkimverify = env_get("DKIMVERIFY");
	ptr = (env_get("RELAYCLIENT") || env_get("AUTHINFO")) ? "" : 0;
	if (dkimverify && ptr && env_get("RELAYCLIENT_NODKIMVERIFY")) {
		execv(*binqqargs, binqqargs);
		die(120, 0);
	}
	if (!dkimsign && !dkimverify && ptr) {
		if (!(dkimsign = env_get("DKIMKEY"))) {
			if (!stralloc_copys(&dkimfn, controldir) ||
					!stralloc_cats(&dkimfn, "control/domainkeys/%/default") ||
					!stralloc_0(&dkimfn))
				die(51, 0);
			dkimsign = dkimfn.s;
		}
	}
	if (dkimsign) {
		if ((ret = control_readfile(&bouncehost, "control/bouncehost", 1)) == -1)
			custom_error("qmail-dkim", "Z", "unable to read bouncehost.", 0, "X.3.0");
		else
		if (ret && !env_put2("BOUNCEDOMAIN", bouncehost.s))
			die(51, 0);
		/* selector */
		ptr = dkimsign;
		selector = ptr;
		while (*ptr) {
			if (*ptr == '/' && *(ptr + 1))
				selector = ptr + 1;
			ptr++;
		}
		str_copyb(sopts.szSelector, selector, sizeof(sopts.szSelector) - 1);

		if (dkim_setoptions(&sopts, env_get("DKIMSIGNOPTIONS")))
			custom_error("qmail-dkim", "Z", "Invalid DKIMSIGNOPTIONS", 0, "X.3.0");
		ptr = env_get("DKIMIDENTITY");
		if (ptr && *ptr)
			str_copyb(sopts.szIdentity, ptr, sizeof(sopts.szIdentity) - 1);
		ptr = env_get("DKIMEXPIRE");
		if (ptr && *ptr)
			sopts.expireTime = starttime + atol(ptr);
		else
		if (ptr)
			sopts.expireTime = 0;
		sopts.pfnHeaderCallback = SignThisHeader;
		if (DKIMSignInit(&ctxt, &sopts) != DKIM_SUCCESS) /*- failed to initialize signature */
			custom_error("qmail-dkim", "Z", "dkim initialization failed", 0, "X.3.0");
	} else {
		char           *x;

		if (!dkimverify)
			dkimverify = "";
		if (!(x = env_get("SIGN_PRACTICE")))
			x = "adsp";
		if (!str_diffn("adsp", x, 4)) {
			useADSP = 1;
			accept3ps = 1;
		} else
		if (!str_diffn("ssp", x, 3)) {
			useSSP = 1;
			accept3ps = 1;
		} else
		if (!str_diffn("local", x, 5)) {
			useSSP = 0;
			useADSP = 0;
			accept3ps = 1;
		}
		if (useADSP)
			vopts.nCheckPractices = useADSP;
		else
		if (useSSP)
			vopts.nCheckPractices = useSSP;
		else
			vopts.nCheckPractices = 0;
		vopts.nAccept3ps = accept3ps;
		if (!(callbackdata = env_get("SELECTOR_DATA")))
			vopts.pfnSelectorCallback = NULL;	/*- SelectorCallback; */
		else
			vopts.pfnSelectorCallback = dns_bypass;
		if (env_get("UNSIGNED_FROM"))
			vopts.nAllowUnsignedFromHeaders = 1;
		vopts.nSubjectRequired = env_get("UNSIGNED_SUBJECT") ? 0 : 1;
		vopts.nHonorBodyLengthTag = env_get("HONOR_BODYLENGTHTAG") ? 0 : 1;
		DKIMVerifyInit(&ctxt, &vopts);		/*- this is always successful */
	}
	/*- Initialization */
	uid = getuid();
	datetime_tai(&dt, starttime);
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);
	mypid = getpid();
	pidopen(); /*- fd = messfd */
	if ((readfd = open_read(pidfn)) == -1)
		die(63, dkimsign ? 1 : 2);
	if (unlink(pidfn) == -1)
		die(63, dkimsign ? 1 : 2);
	substdio_fdbuf(&ssout, write, messfd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, read, 0, inbuf, sizeof(inbuf)); /*- message content */
	for (ret = 0;;) {
		register int    n;
		register char  *x;

		if ((n = substdio_feed(&ssin)) < 0) {
			(dkimsign ? DKIMSignFree : DKIMVerifyFree) (&ctxt);
			die_read();
		}
		if (!n)
			break;
		x = substdio_PEEK(&ssin);
		if (!ret) {
			if ((ret = (dkimsign ? DKIMSignProcess : DKIMVerifyProcess) (&ctxt, x, n)) == DKIM_INVALID_CONTEXT)
				(dkimsign ? DKIMSignFree : DKIMVerifyFree) (&ctxt);
			maybe_die_dkim(ret);
		}
		if (substdio_put(&ssout, x, n) == -1) {
			(dkimsign ? DKIMSignFree : DKIMVerifyFree) (&ctxt);
			die_write();
		}
		substdio_SEEK(&ssin, n);
	}
	if (substdio_flush(&ssout) == -1) {
		(dkimsign ? DKIMSignFree : DKIMVerifyFree) (&ctxt);
		die_write();
	}
	if (dkimsign || dkimverify) {
		if (dkimsign) {
			char           *t;

			if (!(t = DKIMSignGetDomain(&ctxt))) {
				DKIMSignFree(&ctxt);
				maybe_die_dkim(DKIM_INVALID_CONTEXT);
			}
			write_signature(t, &sopts, sizeof(sopts.szSelector) - 1); /*- calls DKIMSignFree(&ctxt) */
		} else
		if (dkimverify) {
			char            szPolicy[512];

			if (!ret) {
				if ((ret = DKIMVerifyResults(&ctxt, &sCount, &sSize)) != DKIM_SUCCESS)
					maybe_die_dkim(ret);
				if ((ret = DKIMVerifyGetDetails(&ctxt, &nSigCount, &pDetails, szPolicy)) != DKIM_SUCCESS)
					maybe_die_dkim(ret);
				else
				for (ret = DKIM_FAIL,i = 0; i < nSigCount; i++) {
					if (pDetails[i].nResult >= 0) {
						ret = 0;
					} else {
						if (ret == DKIM_FAIL)
							ret = pDetails[i].nResult;
					}
				}
				if (!nSigCount)
					ret = DKIM_NO_SIGNATURES;
			}
			/*- what to do if DKIM Verification fails */
			if (checkPractice(ret, useADSP, useSSP)) {
				char           *domain;
				int             skip_nosignature_domain = 0;

				origRet = ret;
				if ((domain = DKIMVerifyGetDomain(&ctxt))) {
					if (!(ptr = env_get("SIGNATUREDOMAINS"))) {
						format_controlfile("signaturedomains", 2);
						if (control_readfile(&sigdomains, fntmp.s, 0) == -1)
							custom_error("qmail-dkim", "Z", "unable to read signaturedomains.", 0, "X.3.0");
					} else
					if (!stralloc_copys(&sigdomains, ptr))
						die(51, 2);
					for (len = 0, ptr = sigdomains.s;len < sigdomains.len;) {
						len += ((token_len = str_len(ptr)) + 1); /*- next domain */
						if (!case_diffb(ptr, token_len, domain)) {
							ret = origRet;
							skip_nosignature_domain = 1;
							useADSP = 0;
							useSSP = 0;
							break;
						}
						ptr = sigdomains.s + len;
					}
					/* if not found in signaturedomains */
					if (!skip_nosignature_domain) {
						if (!(ptr = env_get("NOSIGNATUREDOMAINS"))) {
							format_controlfile("nosignaturedomains", 2);
							if (control_readfile(&nsigdomains, fntmp.s, 0) == -1)
								custom_error("qmail-dkim", "Z", "unable to read nosignaturedomains.", 0, "X.3.0");
						} else
						if (!stralloc_copys(&nsigdomains, ptr))
							die(51, 2);
						for (len = 0, ptr = nsigdomains.s;len < nsigdomains.len;) {
							len += ((token_len = str_len(ptr)) + 1); /*- next domain */
							if (*ptr == '*' || !case_diffb(ptr, token_len, domain)) {
								ret = DKIM_NEUTRAL;
								useADSP = 0;
								useSSP = 0;
								break;
							}
							ptr = nsigdomains.s + len;
						}
					}
				}
				if (!domain || !*domain)
					; /*- do nothing ? */
				else
				if (useADSP) {
					resDKIMADSP = checkADSP(domain);
					if (sCount > 0) {
						if (resDKIMADSP == DKIM_ADSP_UNKNOWN || resDKIMADSP == DKIM_ADSP_ALL)
							ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
					}
					/* if the message should be signed, return fail */
					if (resDKIMADSP == DKIM_ADSP_DISCARDABLE)
						ret = DKIM_FAIL;
					else
						ret = DKIM_NEUTRAL;
				} else
				if (useSSP) {
					int             bTestingPractices = 0;
					char           *domain_t;

					if ((domain_t = DKIMVerifyGetDomain(&ctxt)))
						resDKIMSSP = checkSSP(domain_t, &bTestingPractices);
					if (sCount > 0) {
						if ((resDKIMSSP == DKIM_SSP_UNKNOWN || resDKIMSSP == DKIM_SSP_ALL))
							ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
					}
					// if the SSP is testing, return neutral
					if (bTestingPractices)
						ret = DKIM_NEUTRAL;
					/* if the message should be signed, return fail */
					if (resDKIMSSP == DKIM_SSP_ALL || resDKIMSSP == DKIM_SSP_STRICT)
						ret = DKIM_FAIL;
					else
						ret = DKIM_NEUTRAL;
				}
			}
			DKIMVerifyFree(&ctxt);
			writeHeaderNexit(ret, origRet, resDKIMSSP, resDKIMADSP, useSSP, useADSP);
		} /*- if (dkimverify) */
	}
	if (pipe(pim) == -1)
		die(59, 0);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		die(58, 0);
	case 0:
		close(pim[1]);
		if (fd_move(0, pim[0]) == -1)
			die(120, 0);
		if (!(ptr = env_get("DKIMSIGNEXTRA")))
			execv(*binqqargs, binqqargs);
		else {
			if (!env_put2("NODKIMKEYS", "qmail-dkim") || !env_put2("DKIMSIGN", ptr))
				die(51, 0);
			else
			if ((ptr = env_get("DKIMSIGNOPTIONSEXTRA")) && !env_put2("DKIMSIGNOPTIONS", ptr))
				die(51, 0);
			if (!env_unset("DKIMSIGNEXTRA"))
				die(51, 0);
			execv(argv[0], argv);
		}
		die(120, 0);
	}
	close(pim[0]);
	substdio_fdbuf(&ssin, read, readfd, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, write, pim[1], outbuf, sizeof(outbuf));
	if (substdio_bput(&ssout, dkimoutput.s, dkimoutput.len) == -1) /*- write DKIM signature */
		die_write();
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		die_read();
	case -3:
		die_write();
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	close(pim[1]);
	if (wait_pid(&wstat, pid) != pid)
		die(122, 0);
	if (wait_crashed(wstat))
		die(123, 0);
	die(wait_exitcode(wstat), 0);
}

#ifndef lint
void
getversion_qmail_dkim_c()
{
	static char    *x = "$Id: qmail-dkim.c,v 1.76 2024-01-10 23:01:23+05:30 Cprogrammer Exp mbhangui $";

#ifdef HASDKIM
	x = sccsidmakeargsh;
	x = sccsidqmultih;
	x = sccsidpidopenh;
	x = sccsidgetdomainth;
#endif
	x++;
}
#endif

/*
 * $Log: qmail-dkim.c,v $
 * Revision 1.76  2024-01-10 23:01:23+05:30  Cprogrammer
 * reset sgoptind, sgoptpos for repeated calls to subgetopt
 *
 * Revision 1.75  2024-01-10 10:05:58+05:30  Cprogrammer
 * use bouncehost/me control file if BOUNCEDOMAIN is not set
 * set DKIMSIGN to private key from dkimkeys control file
 *
 * Revision 1.74  2023-11-20 11:03:04+05:30  Cprogrammer
 * Added env variable EXCLUDE_DKIMSIGN to exclude headers from DKIM signing
 * exclude Arc-Authentication-Results header from DKIM signing
 *
 * Revision 1.73  2023-02-17 11:49:48+05:30  Cprogrammer
 * added env variable NODKIMKEYS to disable reading of dkimkeys control file
 * disable dkimkeys when doing DKIMSIGNEXTRA
 *
 * Revision 1.72  2023-02-01 18:15:33+05:30  Cprogrammer
 * use dkimkeys for setting env variables facilitating multi-signature generation with mixed encryption methods
 *
 * Revision 1.71  2023-01-30 10:42:16+05:30  Cprogrammer
 * set pfnSelectorCallback to dns_bypass if SELECTOR_DATA is set
 *
 * Revision 1.70  2023-01-29 22:37:42+05:30  Cprogrammer
 * fixed ed25519 signature
 * added DKIMSIGNEXTRA, DKIMSIGNOPTIONSEXTRA env variables to insert additional signature
 *
 * Revision 1.69  2023-01-26 22:59:19+05:30  Cprogrammer
 * removed setting redundant -b option
 * update verification message to include ED25519 failure
 *
 * Revision 1.68  2022-10-17 19:44:32+05:30  Cprogrammer
 * use exit codes defines from qmail.h
 *
 * Revision 1.67  2022-10-17 12:28:50+05:30  Cprogrammer
 * replace all '%' character with domain name
 *
 * Revision 1.66  2022-10-03 12:28:16+05:30  Cprogrammer
 * fixed return exit codes
 *
 * Revision 1.65  2022-10-02 22:21:48+05:30  Cprogrammer
 * fixed 'Private key file does not exist' for DKIMSIGN with '%'
 *
 * Revision 1.64  2022-04-03 18:44:21+05:30  Cprogrammer
 * refactored qmail_open() error codes
 *
 * Revision 1.63  2022-03-08 22:59:35+05:30  Cprogrammer
 * use custom_error() from custom_error.c
 *
 * Revision 1.62  2021-09-12 14:17:30+05:30  Cprogrammer
 * restore gid after reading private key file
 *
 * Revision 1.61  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.60  2021-08-28 23:16:06+05:30  Cprogrammer
 * control file dkimkeys for domain specific private key, selector
 *
 * Revision 1.59  2021-06-15 22:15:14+05:30  Cprogrammer
 * pass tmpdir argument to pidopen
 *
 * Revision 1.58  2021-06-15 11:53:44+05:30  Cprogrammer
 * moved pidopen out to its own file
 *
 * Revision 1.57  2021-06-09 21:14:33+05:30  Cprogrammer
 * use qmulti() instead of exec of qmail-multi
 *
 * Revision 1.56  2021-05-26 10:44:21+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.55  2020-05-11 11:06:35+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.54  2020-04-01 16:14:36+05:30  Cprogrammer
 * added header for makeargs() function
 *
 * Revision 1.53  2019-06-14 21:26:37+05:30  Cprogrammer
 * added env variable HONOR_BODYLENGTHTAG to honor body length tag during verification
 *
 * Revision 1.52  2019-02-18 22:18:12+05:30  Cprogrammer
 * allow DKIMVERIFY env variable in place of DKIMPRACTICE when SIGN_PRACTICE="local"
 *
 * Revision 1.51  2019-02-17 11:38:51+05:30  Cprogrammer
 * set original DKIM error for SIGN_PRACTICE=local
 *
 * Revision 1.50  2019-02-15 21:25:04+05:30  Cprogrammer
 * skip nosignaturedomains if domain is present in signaturedomains
 *
 * Revision 1.49  2018-08-08 23:58:01+05:30  Cprogrammer
 * issue success if at lease one one good signature is found
 *
 * Revision 1.48  2017-09-05 12:37:16+05:30  Cprogrammer
 * added missing DKIM_MFREE()
 *
 * Revision 1.47  2016-06-03 09:57:59+05:30  Cprogrammer
 * moved qmail-multi to sbin
 *
 * Revision 1.46  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.45  2016-03-01 18:48:02+05:30  Cprogrammer
 * added env variable UNSIGNED_SUBJECT to verify dkim without subject field
 *
 * Revision 1.44  2015-12-15 16:05:58+05:30  Cprogrammer
 * increased buffer size for long header issue
 *
 * Revision 1.43  2014-01-22 22:45:01+05:30  Cprogrammer
 * treat AUTHINFO environment like RELAYCLIENT environment variable
 *
 * Revision 1.42  2013-10-01 17:11:24+05:30  Cprogrammer
 * fixed QMAILQUEUE recursion
 *
 * Revision 1.41  2013-09-16 22:16:35+05:30  Cprogrammer
 * corrected logic for RELAYCLIENT_NODKIMVERIFY
 *
 * Revision 1.40  2013-09-13 16:34:35+05:30  Cprogrammer
 * turn off verification if RELAYCLIENT, DKIMVERIFY and RELAYCLIENT_NODKIMVERIFY is set
 *
 * Revision 1.39  2013-08-18 15:53:30+05:30  Cprogrammer
 * revert back to default verification mode if both dksign, dkverify are not set
 *
 * Revision 1.38  2013-08-17 15:00:33+05:30  Cprogrammer
 * BUG - corrected location of private key when % sign is removed
 *
 * Revision 1.37  2013-01-24 22:37:22+05:30  Cprogrammer
 * BUG (fix by Piotr Gronek) - DKIM_FREE(results) called before call to ParseTagValues()
 * alternate code for DKIMSIGN selector file name
 *
 * Revision 1.36  2012-08-16 08:01:46+05:30  Cprogrammer
 * do not skip X-Mailer headers
 *
 * Revision 1.35  2011-11-10 14:32:08+05:30  Cprogrammer
 * BUG ssout to be assigned only after pidopen
 *
 * Revision 1.34  2011-11-07 09:35:59+05:30  Cprogrammer
 * set ssout, sserr, ssin before executing other functions
 *
 * Revision 1.33  2011-07-29 09:29:17+05:30  Cprogrammer
 * fixed key file name
 *
 * Revision 1.32  2011-07-28 19:36:36+05:30  Cprogrammer
 * BUG - fixed opening of private key with absolute path
 *
 * Revision 1.31  2011-07-22 14:40:05+05:30  Cprogrammer
 * fixed checking of private key file
 *
 * Revision 1.30  2011-06-04 14:49:48+05:30  Cprogrammer
 * remove '%' sign from private key if key not found
 *
 * Revision 1.29  2011-06-04 14:22:29+05:30  Cprogrammer
 * added DKIM_UNSIGNED_FROM error code for dkimpractice
 *
 * Revision 1.28  2011-06-04 14:07:41+05:30  Cprogrammer
 * added DKIM_UNSIGNED_FROM
 *
 * Revision 1.27  2011-02-10 23:39:59+05:30  Cprogrammer
 * use DKIMKEY to override defult control/domainkeys/%/default
 *
 * Revision 1.26  2011-02-06 10:13:50+05:30  Cprogrammer
 * BUG - signature was wrongly freed before being accessed.
 *
 * Revision 1.25  2011-02-05 09:47:47+05:30  Cprogrammer
 * fixed SIGSEGV occuring for messages without body
 *
 * Revision 1.24  2010-11-02 18:45:14+05:30  Cprogrammer
 * Improve DKIM signing/verification speed
 *
 * Revision 1.23  2010-07-21 08:59:57+05:30  Cprogrammer
 * use CONTROLDIR environment variable instead of a hardcoded control directory
 *
 * Revision 1.22  2009-04-22 13:42:51+05:30  Cprogrammer
 * made fd for custom error configurable through env variable ERROR_FD
 *
 * Revision 1.21  2009-04-21 09:05:48+05:30  Cprogrammer
 * return relevant error message for reading private key
 *
 * Revision 1.20  2009-04-21 08:55:41+05:30  Cprogrammer
 * return temporary error for temp failures
 *
 * Revision 1.19  2009-04-20 22:19:01+05:30  Cprogrammer
 * made dkimopts global
 *
 * Revision 1.18  2009-04-16 13:48:32+05:30  Cprogrammer
 * added dkim_setoptions() to set all DKIM options
 *
 * Revision 1.17  2009-04-07 11:36:56+05:30  Cprogrammer
 * use TMPDIR env variable for tmp directory
 *
 * Revision 1.16  2009-04-05 12:52:17+05:30  Cprogrammer
 * added preprocessor warning
 *
 * Revision 1.15  2009-04-04 00:33:44+05:30  Cprogrammer
 * removed dk_strdup()
 *
 * Revision 1.14  2009-03-31 08:21:58+05:30  Cprogrammer
 * set dkimsign when RELAYCLIENT is defined when both dkimsign and dkimverify are undefined
 *
 * Revision 1.13  2009-03-30 22:25:54+05:30  Cprogrammer
 * made DKIM messages friendlier
 *
 * Revision 1.12  2009-03-30 14:47:59+05:30  Cprogrammer
 * added descriptive text for original dkim error
 *
 * Revision 1.11  2009-03-29 19:20:43+05:30  Cprogrammer
 * added nosignaturedomains
 *
 * Revision 1.10  2009-03-28 22:27:02+05:30  Cprogrammer
 * use DKIMSIGN, DKIMVERIFY if RELAYCLIENT is not set
 *
 * Revision 1.9  2009-03-28 22:03:05+05:30  Cprogrammer
 * fixed DKIM return codes
 *
 * Revision 1.8  2009-03-28 13:37:37+05:30  Cprogrammer
 * call DKIMVerifyGetDetails() always
 *
 * Revision 1.7  2009-03-28 11:39:23+05:30  Cprogrammer
 * set automatic setting of dkimsign, dkimverify variables based on RELAYCLIENT
 *
 * Revision 1.6  2009-03-28 11:35:58+05:30  Cprogrammer
 * added ADSP/SSP
 *
 * Revision 1.5  2009-03-22 17:39:38+05:30  Cprogrammer
 * set identity using basename of signature or environment variable DKIMIDENTITY
 *
 * Revision 1.4  2009-03-22 16:58:38+05:30  Cprogrammer
 * fixed bug with verification
 * report custom errors to qmail-queue through custom error interface
 *
 * Revision 1.3  2009-03-21 12:34:38+05:30  Cprogrammer
 * use hasdkim.h for conditional compilation of dkim
 *
 * Revision 1.2  2009-03-20 22:35:57+05:30  Cprogrammer
 * set error to DKIM_NO_SIGNATURE when DKIM-Signature is not present
 *
 * Revision 1.1  2009-03-18 13:54:49+05:30  Cprogrammer
 * Initial revision
 *
 */
