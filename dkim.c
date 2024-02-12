/*
 * This code incorporates intellectual property owned by Yahoo! and licensed
 * pursuant to the Yahoo! DomainKeys Patent License Agreement.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * (cat /tmp/mail.txt|./dkim -z 2 -y private \
 * -s /var/indimail/control/domainkeys/private ;cat /tmp/mail.txt )|./dkim -v
 */
#if 0
#ifndef __cplusplus
#error A C++ compiler is required!
#endif
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dkim.h"
#include "dns_text.h"

#ifdef HAVE_OPENSSL_EVP_H
#include <openssl/evp.h>
#define DKIM_MALLOC(s)     OPENSSL_malloc(s)
#define DKIM_REALLOC(s, n) OPENSSL_realloc((s), (n))
#define DKIM_MFREE(s)      OPENSSL_free(s); s = NULL;
#else
#define DKIM_MALLOC(s)     malloc(s)
#define DKIM_MFREE(s)      free(s); s = NULL;
#endif
#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

static char   *callbackdata, *excl;
const char    *defaultkey = "private";
char           *program;
typedef struct
{
	char           *fn;
	char           *selector;
	int             hash;
	char           *key;
	int             len;
} PRIVKEY;

void
usage()
{
#ifdef HAVE_EVP_SHA256
	fprintf(stderr, "usage: %s [-lqthvVHS] [-p <0|1|2>] [-c <r|s|t|u>] [-d domain]\n%s\n", program,
			"\t[-i you@domain] [-x expire_time] [-z hash] [-y selector] -s privkeyfile [-T dnstext]");
#else
	fprintf(stderr, "usage: %s [-lqthvVHS] [-p <0|1|2>] [-c <r|s|t|u>] [-d domain]\n%s\n", program,
			"\t[-i you@domain] [-x expire_time] [-y selector] -s privkeyfile [-T dnstext]");
#endif
	fprintf(stderr, "l                    include body length tag\n");
	fprintf(stderr, "q                    include query method tag\n");
	fprintf(stderr, "t                    include a timestamp tag\n");
	fprintf(stderr, "h                    include Copied Headers. This adds the z= tag containing\n");
	fprintf(stderr, "                     a copy of the message's original headers.\n");
	fprintf(stderr, "f                    allow Unsigned From (default is to reject if From field is not signed)\n");
	fprintf(stderr, "S                    allow Unsigned Subject (default is to reject if Subject field is not signed)\n");
	fprintf(stderr, "v                    verify the message\n");
	fprintf(stderr, "p <ssp|adsp>         0 - disable practice (default), 1- SSP, or 2 - ADSP verification\n");
	fprintf(stderr, "c <canonicalization> r for relaxed [DEFAULT], s - simple, t relaxed/simple, u - simple/relaxed\n");
	fprintf(stderr, "d <domain>           the domain tag, if not provided, determined from the sender/from header\n");
	fprintf(stderr, "i <identity>         the identity, if not provided it will not be included\n");
	fprintf(stderr, "x <expire_time>      the expire time in seconds since epoch ( DEFAULT = current time + 604800)\n");
	fprintf(stderr, "                     if set to - then it will not be included\n");
#ifdef HAVE_EVP_SHA256
	fprintf(stderr, "z <hash>             1 - sha1, 2 - sha256, 3 - sha1+sha256, 4 - ed25519\n");
#endif
	fprintf(stderr, "y <selector>         the selector tag DEFAULT=basename of privkeyfile\n");
	fprintf(stderr, "s <privkeyfile>      sign the message using the private key in privkeyfile\n");
	fprintf(stderr, "T DNSText            Use DNSText as domainkey text record instead of using DNS\n");
	fprintf(stderr, "X excl               Exclude header excl from signing\n");
	fprintf(stderr, "V                    set verbose mode\n");
	fprintf(stderr, "H                    this help\n");
	exit(1);
}

int DKIM_CALL
SignThisHeader(const char *szHeader)
{
	char           *ptr, *cptr;
	int             i;

	if ((!strncasecmp(szHeader, "X-", 2) && strncasecmp(szHeader, "X-Mailer:", 9))
			|| !strncasecmp(szHeader, "Received:", 9)
			|| !strncasecmp(szHeader, "Authentication-Results:", 23)
			|| !strncasecmp(szHeader, "Arc-Authentication-Results:", 27)
			|| !strncasecmp(szHeader, "DKIM-Signature:", 15)
			|| !strncasecmp(szHeader, "DomainKey-Signature:", 20)
			|| !strncasecmp(szHeader, "Return-Path:", 12))
		return 0;
	if (!excl)
		return 1;
	for (i = 0, cptr = ptr = excl; *ptr; ptr++) {
		if (*ptr == ':') {
			if (strncasecmp((char *) szHeader, cptr, i + 1) == 0)
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

unsigned int str_chr(char *s, int c)
{
	char            ch;
	char           *t;

	ch = c;
	t = s;
	for (;;) {
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
	}
	return t - s;
}

const char *dkim_error_str(int ret, int flag)
{
	switch (ret)
	{
	case DKIM_SUCCESS:			/*- 0 */ /*- A */
		return flag ? "good        " : NULL;
	case DKIM_FAIL:				/*- -1 */ /*- F */
		return flag ? "bad         " : "DKIM Signature verification failed";
	case DKIM_FINISHED_BODY:	/*- 1 process result: no more message body is needed */
		return "process result: no more message body is needed";
	case DKIM_PARTIAL_SUCCESS:	/*- 2 verify result: at least one but not all signatures verified */
		return "verify result: at least one but not all signatures verified";
	case DKIM_NEUTRAL:			/*- 3 verify result: no signatures verified but message is not suspicious */
		return "verify result: no signatures verified but message is not suspicious";
	case DKIM_SUCCESS_BUT_EXTRA:/*- 4 signature result: signature verified but it did not include all of the body */
		return "signature result: signature verified but it did not include all of the body";
	case DKIM_3PS_SIGNATURE:
		break;
	case DKIM_BAD_SYNTAX:		/*- -2 */ /*- G */
		return "signature error: DKIM-Signature could not parse or has bad tags/values";
	case DKIM_SIGNATURE_BAD:	/*- -3 */
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
		return "signature error: RSA/ED25519 verify failed";
#else
		return "signature error: RSA verify failed";
#endif
	case DKIM_SIGNATURE_BAD_BUT_TESTING:
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
		return "signature error: RSA/ED25519 verify failed but testing";
#else
		return "signature error: RSA verify failed but testing";
#endif
	case DKIM_SIGNATURE_EXPIRED:
		return "signature error: x= is old";
	case DKIM_SELECTOR_INVALID:
		return "signature error: selector doesn't parse or contains invalid values";
	case DKIM_SELECTOR_GRANULARITY_MISMATCH:
		return "signature error: selector g= doesn't match i=";
	case DKIM_SELECTOR_KEY_REVOKED:
		return "signature error: selector p= empty";
	case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG:
		return "signature error: selector domain name too long to request";
	case DKIM_SELECTOR_DNS_TEMP_FAILURE:
		return "signature error: temporary dns failure requesting selector";
	case DKIM_SELECTOR_DNS_PERM_FAILURE:
		return "signature error: permanent dns failure requesting selector";
	case DKIM_SELECTOR_PUBLIC_KEY_INVALID:
		return "signature error: selector p= value invalid or wrong format";
	case DKIM_NO_SIGNATURES:
		return "no signatures";
	case DKIM_NO_VALID_SIGNATURES:
		return "no valid signatures";
	case DKIM_BODY_HASH_MISMATCH:
		return "signature verify error: message body does not hash to bh value";
	case DKIM_SELECTOR_ALGORITHM_MISMATCH:
		return "signature error: selector h= doesn't match signature a=";
	case DKIM_STAT_INCOMPAT:
		return "signature error: incompatible v=";
	case DKIM_UNSIGNED_FROM:
		return "signature error: not all message's From headers in signature";
	case DKIM_OUT_OF_MEMORY:
		return "memory allocation failed";
	case DKIM_INVALID_CONTEXT:
		return "DKIMContext structure invalid for this operation";
	case DKIM_NO_SENDER:
		return "signing error: Could not find From: or Sender: header in message";
	case DKIM_BAD_PRIVATE_KEY:
		return "signing error: Could not parse private key";
	case DKIM_BUFFER_TOO_SMALL:
		return "signing error: Buffer passed in is not large enough";
	case DKIM_EVP_SIGN_FAILURE:
		return "signing error: evp signing failure";
	case DKIM_EVP_DIGEST_FAILURE:
		return "signing error: evp digest failure";
	}
	return "unknown error";
}

void
dkim_error(int e)
{
	const char     *ptr;
	
	if ((ptr = dkim_error_str(e, 0)))
		fprintf(stderr, "%s\n", ptr);
	return;
}

/*
 * Allows you to add the headers contain the results and DKIM ADSP
 */
void writeHeader(int ret, int resDKIMSSP, int resDKIMADSP, int useSSP, int useADSP )
{
	char           *dkimStatus, *sspStatus, *adspStatus;

	sspStatus = adspStatus = (char *) "";
	dkimStatus = (char *) dkim_error_str(ret, 1);
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
				adspStatus = (char *) "unknown;";
				break;
		}
	}
	printf("DKIM-Status: %s\n", dkimStatus);
	if (useSSP && *sspStatus)
		printf("X-DKIM-SSP: %s\n", sspStatus);
	if (useADSP && *adspStatus)
		printf("X-DKIM-ADSP: %s\n", adspStatus);
}

int
ParseTagValues(char *list, char *letters[], char *values[])
{
	char           *tmp, *ptr, *key;
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
				if (!strcmp(letters[i], key)) {
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
					tmp = ptr;
					if (*ptr)
						*ptr++ = 0;
					for(;tmp != values[i];tmp--) /*- RFC 4871 3.2 */ {
						if ((*tmp == ' ') || (*tmp == '\t') || (*tmp == '\r') || (*tmp == '\n')) {
							*tmp = 0;
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
GetSSP(char *domain, int *bTesting)
{
	char           *query, *results;
	char           *tags[] = { (char *) "dkim", (char *) "t", (char *) 0};
	char           *values[2];
	int             bIsParentSSP = 0, iSSP = DKIM_SSP_UNKNOWN;

	*bTesting = 0;
	if (!(query = (char *) DKIM_MALLOC(strlen("_ssp._domainkey.") + strlen(domain) + 1))) {
		fprintf(stderr, "malloc: %ld: %s\n", strlen("_ssp._domainkey.") + strlen(domain) + 1,
			strerror(errno));
		exit(1);
	}
	sprintf(query, "_ssp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!strcmp(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_SSP_TEMPFAIL;
	} else
	if (!strcmp(results, "e=perm;")) {
		DKIM_MFREE(results);
		results = dns_text(domain);
		if (!strcmp(results, "e=temp;")) {
			DKIM_MFREE(results);
			return DKIM_SSP_TEMPFAIL;
		} else
		if (!strcmp(results, "e=perm;")) {
			DKIM_MFREE(results);
			return DKIM_SSP_SCOPE;
		}
		bIsParentSSP = 1;
	}
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
	/* flags */
	if (values[1] != NULL) {
		char           *s, *p;
		for (p = values[1], s = values[1]; *p; p++) {
			if (*p == '|')
				*p = 0;
			else
				continue;
			if (!strcmp(s, "y"))
				*bTesting = 1;
			else
			if (!strcmp(s, "s")) {
				if (bIsParentSSP) {
					/*
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
	return iSSP; /*- No ADSP Record */
}

int
GetADSP(char *domain)
{
	char           *query, *results;
	char           *tags[] = {(char *) "dkim", (char *) 0};
	char           *values[1];

	results = dns_text(domain);
	if (!strcmp(results, "e=perm;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	} else
	if (!strcmp(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	if (!(query = (char *) DKIM_MALLOC(strlen((char *) "_adsp._domainkey.") + strlen(domain) + 1))) {
		fprintf(stderr, "malloc: %ld: %s\n", strlen("_adsp._domainkey.") + strlen(domain) + 1,
			strerror(errno));
		exit(1);
	}
	sprintf(query, "_adsp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!strcmp(results, "e=perm;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	} else
	if (!strcmp(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	if (!ParseTagValues(results, tags, values)) {
		DKIM_MFREE(results);
		return DKIM_ADSP_UNKNOWN;
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

int
dns_bypass(const char *domain, char *buffer, int maxlen)
{
	strcpy(buffer, callbackdata);
	return 0;
}

int
mktempfile(int seekfd)
{
	char            filename[sizeof (TMPDIR) + 19] = TMPDIR "/dkim.XXXXXX";
	char            buffer[4096];
	int             fd, ret;

	if (lseek(seekfd, 0, SEEK_SET) == 0)
		return 0;
	if ((fd = mkstemp(filename)) == -1)
		return -1;
	if (unlink(filename))
		return -1;
	for (;;) {
		if ((ret = read(0, buffer, sizeof(buffer) - 1)) == -1) {
			close(fd);
			return -1;
		} else
		if (!ret)
			break;
		if (write(fd, buffer, ret) != ret) {
			close(fd);
			return -1;
		}
	}
	if (fd != seekfd) {
		if (dup2(fd, seekfd) == -1) {
			close(fd);
			return -1;
		}
		close(fd);
	}
	if (lseek(seekfd, 0, SEEK_SET) == -1) {
		close(seekfd);
		return -1;
	}
	return (0);
}

int
main(int argc, char **argv)
{
	PRIVKEY        *PrivKey = NULL;
	int             PrivKeyLen = 0, len, nHash = 0;
	char           *pSig = NULL, *dkimverify;
	int             i, ret, ch, fd, verbose = 0;
	int             bSign = 1, nSigCount = 0, useSSP = 0, useADSP = 0, accept3ps = 0;
	int             sCount = 0, sSize = 0, resDKIMSSP = -1, resDKIMADSP = -1;
	int             nAllowUnsignedFromHeaders = 0;
	int             nAllowUnsignedSubject = 1;
	char           *ptr, *selector = NULL;
	char            buffer[4096], szPolicy[512];
	time_t          t;
	struct stat     statbuf;
	DKIMContext     ctxt;
	DKIMSignOptions   sopts = { 0 };
	DKIMVerifyOptions vopts = { 0 };
	DKIMVerifyDetails *pDetails;

	if (!(program = strrchr(argv[0], '/')))
		program = argv[0];
	else
		program++;
	t = time(0);
#ifdef HAVE_EVP_SHA256
	sopts.nHash = DKIM_HASH_SHA1_AND_SHA256;
#else
	sopts.nHash = DKIM_HASH_SHA1;
#endif
	sopts.nCanon = DKIM_SIGN_RELAXED;
	sopts.nIncludeBodyLengthTag = 0;
	sopts.nIncludeQueryMethod = 0;
	sopts.nIncludeTimeStamp = 0;
	sopts.expireTime = t + 604800;	/* expires in 1 week */
	sopts.nIncludeCopiedHeaders = 0;
	strcpy(sopts.szRequiredHeaders, "NonExistent");
	sopts.pfnHeaderCallback = SignThisHeader;
	excl = getenv("EXCLUDE_DKIMSIGN");
	while (1) {
		if ((ch = getopt(argc, argv, "lqtfhHSvVp:b:c:d:i:s:x:X:y:z:T:")) == -1)
			break;
		switch (ch)
		{
		case 'l': /*- body length tag */
			vopts.nHonorBodyLengthTag = 1;
			sopts.nIncludeBodyLengthTag = 1;
			break;
		case 'q': /*- query method tag */
			sopts.nIncludeQueryMethod = 1;
			break;
		case 'S':
			nAllowUnsignedSubject = 0;
			break;
		case 'f':
			nAllowUnsignedFromHeaders = 1;
		case 't': /*- timestamp tag */
			sopts.nIncludeTimeStamp = 1;
			break;
		case 'h':
			sopts.nIncludeCopiedHeaders = 1;
			break;
		case 'H':
			usage();
			break;
		case 'v': /*- verify */
			bSign = 0;
			break;
		case 'V': /*- verbose */
			vopts.verbose = 1;
			sopts.verbose = 1;
			verbose = 1;
			break;
		case 'p':
			switch(*optarg)
			{
			case '1':
				accept3ps = 1;
				useSSP = 1;
				useADSP = 0;
				break;
			case '2':
				accept3ps = 1;
				useSSP = 0;
				useADSP = 1;
				break;
			case '3':
				accept3ps = 1;
				useSSP = 0;
				useADSP = 0;
				break;
			case '0':
				accept3ps = 0;
				useSSP = 0;
				useADSP = 0;
				break;
			default:
				fprintf(stderr, "%s: unrecognized practice %c.\n", program, *optarg);
				return (1);
			}
			break;
		case 'b': /*- dummy option kept for backward compatibility */
			break;
		case 'c':
			switch (*optarg)
			{
			case 'r':
				sopts.nCanon = DKIM_SIGN_RELAXED;
				break;
			case 's':
				sopts.nCanon = DKIM_SIGN_SIMPLE;
				break;
			case 't':
				sopts.nCanon = DKIM_SIGN_RELAXED_SIMPLE;
				break;
			case 'u':
				sopts.nCanon = DKIM_SIGN_SIMPLE_RELAXED;
				break;
			default:
				fprintf(stderr, "%s: unrecognized canonicalization.\n", program);
				return (1);
			}
			break;
		case 'd':
			strncpy(sopts.szDomain, optarg, sizeof (sopts.szDomain) - 1);
			break;
		case 'i':	/*- identity */
			if (*optarg == '-')
				sopts.szIdentity[0] = '\0';
			else
				strncpy(sopts.szIdentity, optarg, sizeof (sopts.szIdentity) - 1);
			break;
		case 'y':
			selector = optarg;
			break;
		case 'z': /*- sign w/ sha1, sha256 or both */
			switch (*optarg)
			{
			case '1':
				nHash = DKIM_HASH_SHA1;
				break;
#ifdef HAVE_EVP_SHA256
			case '2':
				nHash = DKIM_HASH_SHA256;
				break;
			case '3':
				nHash = DKIM_HASH_SHA1_AND_SHA256;
				break;
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
			case '4':
				nHash = DKIM_HASH_ED25519;
				break;
#endif
			default:
				fprintf(stderr, "%s: unrecognized hash.\n", program);
				return (1);
			}
			break;
		case 'x': /*- expire time */
			if (*optarg == '-')
				sopts.expireTime = 0;
			else
				sopts.expireTime = t + atoi(optarg);
			break;
		case 's': /*- sign */
			if (!PrivKeyLen)
				PrivKey = (PRIVKEY *) DKIM_MALLOC(sizeof(PRIVKEY));
			else
				PrivKey = (PRIVKEY *) DKIM_REALLOC(PrivKey, sizeof(PRIVKEY) * (PrivKeyLen + 1));
			if (!PrivKey)
				fprintf(stderr, "malloc: %ld bytes: %s\n", sizeof(PRIVKEY) * PrivKeyLen, strerror(errno));

			if ((fd = open(optarg, O_RDONLY)) == -1) {
				fprintf(stderr, "%s: %s\n", optarg, strerror(errno));
				return (1);
			}
			if (fstat(fd, &statbuf) == -1) {
				fprintf(stderr, "fstat: %s: %s\n", optarg, strerror(errno));
				return (1);
			}
			if (!(ptr = (char *) DKIM_MALLOC(sizeof(char) * ((len = statbuf.st_size) + 1)))) {
#ifdef DARWIN
				fprintf(stderr, "malloc: %lld bytes: %s\n", statbuf.st_size + 1, strerror(errno));
#else
				fprintf(stderr, "malloc: %ld bytes: %s\n", statbuf.st_size + 1, strerror(errno));
#endif
				return (1);
			}
			if (read(fd, ptr, len) != len) {
				fprintf(stderr, "%s: read: %s\n", strerror(errno), program);
				return (1);
			}
			close(fd);
			ptr[len] = '\0';
			PrivKey[PrivKeyLen].fn = optarg;
			PrivKey[PrivKeyLen].key = ptr;
			PrivKey[PrivKeyLen].len = len;
			if (selector)
				PrivKey[PrivKeyLen].selector = selector;
			else
			if ((ptr = strrchr(optarg, '/'))) {
				ptr++;
				PrivKey[PrivKeyLen].selector = ptr;
			} else
				PrivKey[PrivKeyLen].selector = (char *) defaultkey;
			PrivKey[PrivKeyLen].hash = nHash ? nHash : DKIM_HASH_SHA256;
			PrivKeyLen++;
			bSign = 1;
			selector = NULL;
			nHash = 0;
			break;
		case 'T':
			callbackdata = optarg;
			break;
		case 'X':
			excl = optarg;
			break;
		} /*- switch (ch) */
	}
	if (bSign) { /*- sign */
		if (!PrivKey) {
			fprintf(stderr, "Private Key(s) not provided\n");
			usage();
			return (1);
		}
		if (PrivKeyLen > 1 && mktempfile(0)) {
			fprintf(stderr, "unable to make descriptor 0 seekable\n");
			return 1;
		}
		for (i = 0; i < PrivKeyLen; i++) {
			strcpy(sopts.szSelector, PrivKey[i].selector);
			sopts.nHash = PrivKey[i].hash;
			if (DKIMSignInit(&ctxt, &sopts) != DKIM_SUCCESS) {
				fprintf(stderr, "DKIMSignInit: failed to initialize signature %s\n", PrivKey[i].fn);
				return (1);
			}
			/* process mail on descriptor 0 */
			if (PrivKeyLen > 1 && lseek(0, 0, SEEK_SET) == -1) {
				fprintf(stderr, "lseek: descriptor 0: %s\n", strerror(errno));
				return 1;
			}
			for (;;) {
				if ((ret = read(0, buffer, sizeof(buffer) - 1)) == -1) {
					fprintf(stderr, "read: %s\n", strerror(errno));
					DKIMSignFree(&ctxt);
					return (1);
				} else
				if (!ret)
					break;
				buffer[ret] = 0;
				if ((ret = DKIMSignProcess(&ctxt, buffer, ret)) != DKIM_SUCCESS) {
					dkim_error(ret);
					DKIMSignFree(&ctxt);
					return (1);
				}
			}
			if ((ret = DKIMSignGetSig2(&ctxt, PrivKey[i].key, &pSig)) != DKIM_SUCCESS) {
				dkim_error(ret);
				DKIMSignFree(&ctxt);
				return (1);
			}
			if (pSig) {
				fwrite(pSig, 1, strlen(pSig), stdout);
				fwrite("\n", 1, 1, stdout);
			}
			DKIMSignFree(&ctxt);
		}
		return 0;
	} else { /*- verify */
		if (useADSP)
			vopts.nCheckPractices = useADSP;
		else
		if (useSSP)
			vopts.nCheckPractices = useSSP;
		else
			vopts.nCheckPractices = 0;
		vopts.nAccept3ps = accept3ps;
		if (callbackdata)
			vopts.pfnSelectorCallback = dns_bypass; /*- SelectorCallback; */
		else
			vopts.pfnSelectorCallback = NULL; /*- SelectorCallback; */
		vopts.nAllowUnsignedFromHeaders = nAllowUnsignedFromHeaders;
		vopts.nSubjectRequired = nAllowUnsignedSubject;
		DKIMVerifyInit(&ctxt, &vopts); /*- this is always successful */
		for (;;) {
			if ((i = read(0, buffer, sizeof(buffer) - 1)) == -1) {
				fprintf(stderr, "read: %s\n", strerror(errno));
				DKIMVerifyFree(&ctxt);
				return (1);
			} else
			if (!i)
				break;
			if ((ret = DKIMVerifyProcess(&ctxt, buffer, i)))
				dkim_error(ret);
			if (ret > 0 && ret < DKIM_FINISHED_BODY)
				ret = DKIM_FAIL;
			if (ret)
				break;
		}
		if (!ret) {
			ret = DKIMVerifyResults(&ctxt, &sCount, &sSize);
			if (ret != DKIM_SUCCESS && ret != DKIM_3PS_SIGNATURE && ret != DKIM_NEUTRAL)
				dkim_error(ret);
			if ((ret = DKIMVerifyGetDetails(&ctxt, &nSigCount, &pDetails, szPolicy)) != DKIM_SUCCESS)
				dkim_error(ret);
			else {
				for (ret = DKIM_FAIL, i = 0; i < nSigCount; i++) {
					if (verbose)
						fprintf(stderr, "Signature # %02d: ", i + 1);
					if (pDetails[i].nResult >= 0) {
						ret = 0;
						if (verbose)
							fprintf(stderr, "Success\n");
						continue;
					} else {
						if (ret == DKIM_FAIL)
							ret = pDetails[i].nResult;
						if (verbose)
							fprintf(stderr, "Failure %d\n", pDetails[i].nResult);
					}
				}
				if (!nSigCount)
					ret = DKIM_NO_SIGNATURES;
			}
		}
		if (ret < 0 || ret == DKIM_3PS_SIGNATURE) {
			if (useADSP) {
				char           *domain;
	
				if ((domain = DKIMVerifyGetDomain(&ctxt)))
					resDKIMADSP = GetADSP(domain);
				if (sCount > 0) {
					if (resDKIMADSP == DKIM_ADSP_UNKNOWN || resDKIMADSP == DKIM_ADSP_ALL)
						ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
				}
				/* if the message should be signed, return fail */
				if (resDKIMADSP == DKIM_ADSP_DISCARDABLE)
					ret = DKIM_FAIL;
				ret = DKIM_NEUTRAL;
			} else
			if (useSSP) {
				int             bTestingPractices = 0;
				char           *domain;

				if ((domain = DKIMVerifyGetDomain(&ctxt)))
					resDKIMSSP = GetSSP(domain, &bTestingPractices);
				if (sCount > 0) {
					if ((resDKIMSSP == DKIM_SSP_UNKNOWN || resDKIMSSP == DKIM_SSP_ALL))
						ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
				}
				/* if the SSP is testing, return neutral */
				if (bTestingPractices)
					return(DKIM_NEUTRAL);
				/* if the message should be signed, return fail */
				if (resDKIMSSP == DKIM_SSP_ALL || resDKIMSSP == DKIM_SSP_STRICT)
					return(DKIM_FAIL);
				ret = DKIM_NEUTRAL;
			}
		}
		DKIMVerifyFree(&ctxt);
		writeHeader(ret, resDKIMSSP, resDKIMADSP, useSSP, useADSP);
		if ((dkimverify = getenv("DKIMVERIFY"))) {
			if (ret < 0) {
				if (dkimverify[str_chr(dkimverify, 'F' - ret)])
					ret = 14; /*- return permanent error */
				else
				if (dkimverify[str_chr(dkimverify, 'f' - ret)])
					ret = 88; /*- return temporary error */
				else
					ret = 0;
			} else {
				if (dkimverify[str_chr(dkimverify, 'A' + ret)])
					ret = 14; /*- return permanent error */
				else
				if (dkimverify[str_chr(dkimverify, 'a' + ret)])
					ret = 88; /*- return temporary error */
				else
					ret = 0;
			}
		}
		return (ret);
	}
	/*- Not Reached */
	_exit(0);
}

void
getversion_dkim_c()
{
	static char    *x = (char *) "$Id: dkim.cpp,v 1.33 2023-02-12 08:07:00+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: dkim.cpp,v $
 * Revision 1.33  2023-02-12 08:07:00+05:30  Cprogrammer
 * added dkim_error_str function to return DKIM error
 *
 * Revision 1.32  2023-02-04 11:56:10+05:30  Cprogrammer
 * generate DKIM-Signature for each -s option
 *
 * Revision 1.31  2023-01-30 10:41:09+05:30  Cprogrammer
 * set verbose flag for dkimvery, dkimsign methods
 *
 * Revision 1.30  2023-01-27 19:38:28+05:30  Cprogrammer
 * fixed openssl version for ed25519
 *
 * Revision 1.29  2023-01-27 17:11:41+05:30  Cprogrammer
 * removed -b option. Option kept for backward compatibility
 * added -z 4 for setting Ed25519 DKIM signature
 *
 * Revision 1.28  2022-11-27 09:41:38+05:30  Cprogrammer
 * updated help message for -h option
 *
 * Revision 1.27  2021-07-20 23:20:09+05:30  Cprogrammer
 * removed use of register
 *
 * Revision 1.26  2020-10-01 14:14:34+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.25  2020-06-08 23:16:27+05:30  Cprogrammer
 * quench compiler warnings
 *
 * Revision 1.24  2019-06-24 23:14:33+05:30  Cprogrammer
 * fixed return value interpretation of DKIMVERIFY
 *
 * Revision 1.23  2019-06-14 21:24:59+05:30  Cprogrammer
 * BUG - honor body length tag in verification
 *
 * Revision 1.22  2019-01-13 10:10:27+05:30  Cprogrammer
 * added missing usage string for allowing unsigned subject.
 *
 * Revision 1.21  2018-08-08 23:57:02+05:30  Cprogrammer
 * issue success if at lease one one good signature is found
 *
 * Revision 1.20  2018-05-22 10:03:26+05:30  Cprogrammer
 * changed return type of writeHeader() to void
 *
 * Revision 1.19  2016-03-01 16:23:38+05:30  Cprogrammer
 * added -S option to allow email with unsigned subject
 *
 * Revision 1.18  2016-02-01 10:53:32+05:30  Cprogrammer
 * use basename of private key as the selector in absense of -y option
 *
 * Revision 1.17  2015-12-15 15:36:01+05:30  Cprogrammer
 * added case 3 for 3rd party signature without SSP and ADSP
 * increased buffer size for Apple mail with X-BrightMail-Tracker header issue
 *
 * Revision 1.16  2012-08-16 08:01:19+05:30  Cprogrammer
 * do not skip X-Mailer headers
 *
 * Revision 1.15  2011-06-04 13:55:50+05:30  Cprogrammer
 * set AllowUnsignedFromHeaders
 *
 * Revision 1.14  2011-06-04 09:36:36+05:30  Cprogrammer
 * added AllowUnsignedFromHeaders option
 *
 * Revision 1.13  2011-02-07 22:05:23+05:30  Cprogrammer
 * added case DKIM_3PS_SIGNATURE
 *
 * Revision 1.12  2010-05-04 14:00:13+05:30  Cprogrammer
 * make option '-z' work on systems without SHA_256
 *
 * Revision 1.11  2009-04-20 08:35:45+05:30  Cprogrammer
 * corrected usage()
 *
 * Revision 1.10  2009-04-15 21:30:32+05:30  Cprogrammer
 * added DKIM-Signature to list of excluded headers
 *
 * Revision 1.9  2009-04-15 20:45:04+05:30  Cprogrammer
 * corrected usage
 *
 * Revision 1.8  2009-04-05 19:04:44+05:30  Cprogrammer
 * improved formating of usage
 *
 * Revision 1.7  2009-04-03 12:05:25+05:30  Cprogrammer
 * minor changes on usage display
 *
 * Revision 1.6  2009-03-28 20:15:23+05:30  Cprogrammer
 * invoke DKIMVerifyGetDetails()
 *
 * Revision 1.5  2009-03-27 20:43:48+05:30  Cprogrammer
 * added HAVE_OPENSSL_EVP_H conditional
 *
 * Revision 1.4  2009-03-27 20:19:28+05:30  Cprogrammer
 * added ADSP
 *
 * Revision 1.3  2009-03-26 15:10:53+05:30  Cprogrammer
 * added ADSP
 *
 * Revision 1.2  2009-03-25 08:37:45+05:30  Cprogrammer
 * added dkim_error
 *
 * Revision 1.1  2009-03-21 08:24:47+05:30  Cprogrammer
 * Initial revision
 */
