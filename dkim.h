/*
 *  Copyright 2005 Alt-N Technologies, Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  This code incorporates intellectual property owned by Yahoo! and licensed
 *  pursuant to the Yahoo! DomainKeys Patent License Agreement.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#define DKIM_CALL
#define MAKELONG(a,b) ((long)(((unsigned)(a) & 0xffff) | (((unsigned)(b) & 0xffff) << 16)))

#ifdef __cplusplus
extern          "C" {
#endif

#include <time.h>

/* DKIM encryption method rsa or ed25519 */
#define DKIM_ENCRYPTION_RSA         1
#define DKIM_ENCRYPTION_ED25519     2

/* DKIM hash algorithms */
#define DKIM_HASH_SHA1              1
#define DKIM_HASH_SHA256            2
#define DKIM_HASH_SHA1_AND_SHA256   3
#define DKIM_HASH_ED25519           4

/* DKIM canonicalization methods */
#define DKIM_CANON_SIMPLE           1
#define DKIM_CANON_NOWSP            2
#define DKIM_CANON_RELAXED          3

#define DKIM_SIGN_SIMPLE			MAKELONG(DKIM_CANON_SIMPLE,DKIM_CANON_SIMPLE)
#define DKIM_SIGN_SIMPLE_RELAXED	MAKELONG(DKIM_CANON_RELAXED,DKIM_CANON_SIMPLE)
#define DKIM_SIGN_RELAXED			MAKELONG(DKIM_CANON_RELAXED,DKIM_CANON_RELAXED)
#define DKIM_SIGN_RELAXED_SIMPLE	MAKELONG(DKIM_CANON_SIMPLE,DKIM_CANON_RELAXED)

/*
 * DKIM_SUCCESS                                 verify result: all signatures verified
 * signature result: signature verified
 */
#define DKIM_SUCCESS                          0 /* operation successful */
#define DKIM_FINISHED_BODY                    1 /* process result: no more message body is needed */
#define DKIM_PARTIAL_SUCCESS                  2 /* verify result: at least one but not all signatures verified */
#define DKIM_NEUTRAL                          3 /* verify result: no signatures verified but message is not suspicous */
#define DKIM_SUCCESS_BUT_EXTRA                4 /* signature result: signature verified but it did not include all of the body */
#define DKIM_3PS_SIGNATURE                    5 /* 3rd-party signature */

/* DKIM Error codes */
#define DKIM_FAIL                            -1 /* verify error: message is suspicious */
#define DKIM_BAD_SYNTAX                      -2 /* signature error: DKIM-Signature could not parse or has bad tags/values */
#define DKIM_SIGNATURE_BAD                   -3 /* signature error: RSA/ED25519 verify failed */
#define DKIM_SIGNATURE_BAD_BUT_TESTING       -4 /* signature error: RSA/ED25519 verify failed but testing */
#define DKIM_SIGNATURE_EXPIRED               -5 /* signature error: x= is old */
#define DKIM_SELECTOR_INVALID                -6 /* signature error: selector doesn't parse or contains invalid values */
#define DKIM_SELECTOR_GRANULARITY_MISMATCH   -7 /* signature error: selector g= doesn't match i= */
#define DKIM_SELECTOR_KEY_REVOKED            -8 /* signature error: selector p= empty */
#define DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG   -9 /* signature error: selector domain name too long to request */
#define DKIM_SELECTOR_DNS_TEMP_FAILURE      -10 /* signature error: temporary dns failure requesting selector */
#define DKIM_SELECTOR_DNS_PERM_FAILURE      -11 /* signature error: permanent dns failure requesting selector */
#define DKIM_SELECTOR_PUBLIC_KEY_INVALID    -12 /* signature error: selector p= value invalid or wrong format */
#define DKIM_NO_SIGNATURES                  -13 /* process error, no sigs */
#define DKIM_NO_VALID_SIGNATURES            -14 /* process error, no valid sigs */
#define DKIM_BODY_HASH_MISMATCH             -15 /* sigature verify error: message body does not hash to bh value */
#define DKIM_SELECTOR_ALGORITHM_MISMATCH    -16 /* signature error: selector h= doesn't match signature a= */
#define DKIM_STAT_INCOMPAT                  -17 /* signature error: incompatible v= */
#define DKIM_UNSIGNED_FROM                  -18 /* signature error: not all message's From headers in signature */
#define DKIM_OUT_OF_MEMORY                  -19 /* memory allocation failed */
#define DKIM_INVALID_CONTEXT                -20 /* DKIMContext structure invalid for this operation */
#define DKIM_NO_SENDER                      -21 /* signing error: Could not find From: or Sender: header in message */
#define DKIM_BAD_PRIVATE_KEY                -22 /* signing error: Could not parse private key */
#define DKIM_BUFFER_TOO_SMALL               -23 /* signing error: Buffer passed in is not large enough */
#define DKIM_EVP_SIGN_FAILURE               -24 /* signing error: evp signing failure */
#define DKIM_EVP_DIGEST_FAILURE             -25 /* signing error: evp digest failure */
#define DKIM_MAX_ERROR                      -26 /* set this to 1 greater than the highest error code (but negative) */

#define DKIM_SSP_UNKNOWN                      1 /*- some messages may be signed */
#define DKIM_SSP_ALL                          2 /*- all messages are signed, 3rd party allowed */
#define DKIM_SSP_STRICT                       3 /*- all messages are signed, no 3rd party allowed */
#define DKIM_SSP_SCOPE                        4 /*- the domain should be considered invalid */
#define DKIM_SSP_TEMPFAIL                     5 /*- Temporary Error */

#define DKIM_ADSP_UNKNOWN                     1 /*- some messages may be signed */
#define DKIM_ADSP_ALL                         2 /*- All message are signed with author signature */
#define DKIM_ADSP_DISCARDABLE                 3 /*- messages which fail verification are Discardable */
#define DKIM_ADSP_SCOPE                       4 /*- domain is out of scope */
#define DKIM_ADSP_TEMPFAIL                    5 /*- Temporary Error */


/* This function is called once for each header in the message */
/* return 1 to include this header in the signature and 0 to exclude. */
typedef int     (DKIM_CALL *DKIMHEADERCALLBACK) (const char *szHeader);

/* This function is called to retrieve a TXT record from DNS */
typedef int     (DKIM_CALL *DKIMDNSCALLBACK) (const char *szFQDN, char *szBuffer, int nBufLen);

typedef struct DKIMContext_t {
	unsigned int    reserved1;
	unsigned int    reserved2;
	void           *reserved3;
} DKIMContext;

typedef struct DKIMSignOptions_t {
	int             nCanon;                 /* canonization */
	int             nIncludeBodyLengthTag;  /* 0 = don't include l= tag, 1 = include l= tag */
	int             nIncludeTimeStamp;      /* 0 = don't include t= tag, 1 = include t= tag */
	int             nIncludeQueryMethod;    /* 0 = don't include q= tag, 1 = include q= tag */
	char            szSelector[80];         /* selector - required */
	char            szDomain[256];          /* domain - optional - if empty, domain is computed from sender */
	char            szIdentity[256];        /* for i= tag, if empty tag will not be included in sig */
	time_t          expireTime;             /* for x= tag, if 0 tag will not be included in sig */
	DKIMHEADERCALLBACK pfnHeaderCallback;   /* header callback */
	char            szRequiredHeaders[256]; /* colon-separated list of headers that must be signed */
	int             nHash;                  /* use one of the DKIM_HASH_xx constants here, even if not present in the message */
	int             nIncludeCopiedHeaders;  /* 0 = don't include z= tag, 1 = include z= tag */
	int             nIncludeBodyHash;       /* use one of the DKIM_BODYHASH_xx constants here */
	int             verbose;
} DKIMSignOptions;

typedef struct DKIMVerifyOptions_t {
	DKIMDNSCALLBACK pfnSelectorCallback;       /* selector record callback */
	DKIMDNSCALLBACK pfnPracticesCallback;      /* SSP record callback */
	int             nHonorBodyLengthTag;       /* 0 = ignore l= tag, 1 = use l= tag to limit the amount of body verified */
	int             nCheckPractices;           /* 0 = use default (unknown) practices, 1 = request and use sender's signing practices */
	int             nSubjectRequired;          /* 0 = subject is required to be signed, 1 = not required */
	int             nSaveCanonicalizedData;    /* 0 = canonicalized data is not saved, 1 = canonicalized data is saved */
	int             nAllowUnsignedFromHeaders; /* 0 = From headers not included in the signature are not allowed, 1 = allowed */
	int             nAccept3ps;                /* 0 = don't check 3rd party signature(s), 1 = check 3rd party signature(s) */
	int             verbose;
} DKIMVerifyOptions;

typedef struct DKIMVerifyDetails_t {
	char           *szSignature;
	char           *DNS;
	char           *szSignatureDomain;
	char           *szIdentityDomain;
	char           *szCanonicalizedData;
	int             nResult;
} DKIMVerifyDetails;

int DKIM_CALL   DKIMSignInit(DKIMContext *pSignContext, DKIMSignOptions *pOptions);
int DKIM_CALL   DKIMSignProcess(DKIMContext *pSignContext, char *szBuffer, int nBufLength);
int DKIM_CALL   DKIMSignGetSig(DKIMContext *pSignContext, char *szPrivKey, char *szSignature, int nSigLength);
int DKIM_CALL   DKIMSignGetSig2(DKIMContext *pSignContext, char *szPrivKey, char **pszSignature);
void DKIM_CALL  DKIMSignFree(DKIMContext *pSignContext);
char           *DKIM_CALL DKIMSignGetDomain(DKIMContext *pSignContext);

int DKIM_CALL   DKIMVerifyInit(DKIMContext *pVerifyContext, DKIMVerifyOptions *pOptions);
int DKIM_CALL   DKIMVerifyProcess(DKIMContext *pVerifyContext, char *szBuffer, int nBufLength);
int DKIM_CALL   DKIMVerifyResults(DKIMContext *pVerifyContext , int *sCount, int *sSize);
int DKIM_CALL   DKIMVerifyGetDetails(DKIMContext *pVerifyContext, int *nSigCount, DKIMVerifyDetails **pDetails, char *szPractices);
char           *DKIM_CALL DKIMVerifyGetDomain(DKIMContext *pVerifyContext);
void DKIM_CALL  DKIMVerifyFree(DKIMContext *pVerifyContext);
char           *DKIM_CALL DKIMVersion();
char           *DKIM_CALL DKIMGetErrorString(int ErrorCode);
int  DKIM_CALL  DKIMSignReplaceSelector(DKIMContext *pSignContext, DKIMSignOptions *pOptions);
int  DKIM_CALL  DKIMSignReplaceHash(DKIMContext *pSignContext, DKIMSignOptions *pOptions);
#include "macros.h"
#ifdef __cplusplus
}
#endif

/*
 * $Log: dkim.h,v $
 * Revision 1.13  2023-02-11 22:50:12+05:30  Cprogrammer
 * added DKIM_EVP_SIGN_FAILURE, DKIM_EVP_DIGEST_FAILURE definitions for EVP signing and digest failures
 *
 * Revision 1.12  2023-02-01 18:02:45+05:30  Cprogrammer
 * new function DKIMSignReplaceHash to alter current Hash method
 *
 * Revision 1.11  2023-01-29 22:03:53+05:30  Cprogrammer
 * added defines for encryption methods
 *
 * Revision 1.10  2023-01-26 22:38:16+05:30  Cprogrammer
 * added definition for DKIM_HASH_ED25519
 *
 * Revision 1.9  2021-08-28 21:41:40+05:30  Cprogrammer
 * added DKIMSignReplaceSelector to replace selector
 *
 * Revision 1.8  2015-12-15 16:03:18+05:30  Cprogrammer
 * use time_t for expireTime
 *
 * Revision 1.7  2011-06-04 13:56:06+05:30  Cprogrammer
 * corrected return codes
 *
 * Revision 1.6  2011-06-04 10:04:00+05:30  Cprogrammer
 * unified error code for signing & verifcation
 * added signature and identity domain information to
 *     DKIMVerifyDetails structure
 *
 * Revision 1.5  2009-03-27 20:19:05+05:30  Cprogrammer
 * major changes made for incorporating ADSP
 *
 * Revision 1.4  2009-03-26 19:28:15+05:30  Cprogrammer
 * removed DKIM_3PS_PARTIAL_SUCCESS
 *
 * Revision 1.3  2009-03-26 15:11:33+05:30  Cprogrammer
 * added ADSP
 *
 * Revision 1.2  2009-03-25 08:37:58+05:30  Cprogrammer
 * changed definitions of constants to avoid clash between error and success
 *
 * Revision 1.1  2009-03-21 08:50:19+05:30  Cprogrammer
 * Initial revision
 */
