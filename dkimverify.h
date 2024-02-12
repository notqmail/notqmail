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

#ifndef DKIMVERIFY_H
#define DKIMVERIFY_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "dkimbase.h"
#include <vector>

#define DKIM_SIG_VERSION_PRE_02			0
#define DKIM_SIG_VERSION_02_PLUS		1

class           SelectorInfo {
 public:
	SelectorInfo(const string &sSelector, const string &sDomain);
	~SelectorInfo();

	string          Domain;
	string          Selector;
	string          Granularity;
	bool            AllowSHA1;
	bool            AllowSHA256;
	EVP_PKEY       *PublicKey;	/* the public key */
	bool            Testing;
	bool            SameDomain;
	int             method; /* rsa or ed25519 */
	int             Status;
	int             Parse(char *Buffer);
};

class           SignatureInfo {
public:
	SignatureInfo(bool SaveCanonicalizedData);
	~SignatureInfo();

	void            Hash(const char *szBuffer, unsigned nBufLength, bool IsBody = false);
	string          Header;
	unsigned        Version;
	string          Domain;
	string          Selector;
	string          SignatureData;
	string          BodyHashData;
	string          IdentityLocalPart;
	string          IdentityDomain;
	string          CanonicalizedData;
	vector <string> SignedHeaders;
	long            BodyLength;
	unsigned        HeaderCanonicalization;
	unsigned        BodyCanonicalization;
#if SIZEOF_TIME_T  == 8
	time_t          ExpireTime;
#else
	long long       ExpireTime;
#endif
	long            VerifiedBodyCount;
	long            UnverifiedBodyCount;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_MD_CTX     *m_Hdr_ctx = NULL;
	EVP_MD_CTX     *m_Bdy_ctx = NULL;
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
	EVP_MD_CTX     *m_Msg_ctx = NULL;
#endif
#else
	EVP_MD_CTX      m_Hdr_ctx;
	EVP_MD_CTX      m_Bdy_ctx;
#endif
	SelectorInfo   *m_pSelector;
	int             Status;
	int             m_nHash;	/* use one of the DKIM_HASH_xxx constants here */
	unsigned        EmptyLineCount;
	bool            m_SaveCanonicalizedData;
};

class           CDKIMVerify:public CDKIMBase {
public:

	CDKIMVerify();
	~CDKIMVerify();

	int             Init(DKIMVerifyOptions * pOptions);
	int             GetResults(int *sCount, int *sSize);
	int             GetDetails(int *nSigCount, DKIMVerifyDetails ** pDetails);
	virtual int     ProcessHeaders(void);
	virtual int     ProcessBody(char *szBuffer, int nBufLength, bool bEOF);
	const char     *GetPractices() {return Practices.c_str();}
	char           *DKIM_CALL GetDomain(void);

protected:
	int             ParseDKIMSignature(const string &sHeader, SignatureInfo &sig);
	SelectorInfo   &GetSelector(const string &sSelector, const string &sDomain);
	int             GetADSP(const string &sDomain, int &iADSP);
	int             GetSSP(const string &sDomain, int &iSSP, bool &bTesting);
	list <SignatureInfo> Signatures;
	list <SelectorInfo> Selectors;
	DKIMDNSCALLBACK m_pfnSelectorCallback;	/* selector record callback */
	DKIMDNSCALLBACK m_pfnPracticesCallback;	/* SSP record callback */
	bool            m_HonorBodyLengthTag;
	bool            m_CheckPractices;
	bool            m_Accept3ps;		/* TBS(Luc) : accept 3rd party signature(s) */
	bool            m_SubjectIsRequired;
	bool            m_SaveCanonicalizedData;
	bool            m_AllowUnsignedFromHeaders;
	vector <DKIMVerifyDetails> Details;
	string          Practices;
};

#endif	/*- DKIMVERIFY_H */

/*
 * $Log: dkimverify.h,v $
 * Revision 1.12  2023-01-29 22:06:08+05:30  Cprogrammer
 * added new member 'method'
 *
 * Revision 1.11  2023-01-27 19:39:01+05:30  Cprogrammer
 * fixed openssl version for ed25519
 *
 * Revision 1.10  2023-01-26 22:46:59+05:30  Cprogrammer
 * added ed25519 signatures
 *
 * Revision 1.9  2019-06-14 21:25:11+05:30  Cprogrammer
 * BUG - honor body length tag in verification. Changed data type for BodyLength
 *
 * Revision 1.8  2019-05-22 11:30:06+05:30  Cprogrammer
 * fix for 32 bit systems where time_t is 4 bytes & encounters year 2038 issue
 *
 * Revision 1.7  2017-08-31 17:07:45+05:30  Cprogrammer
 * fixed g++ compiler warning
 *
 * Revision 1.6  2017-08-09 22:03:46+05:30  Cprogrammer
 * initialized EVP_MD_CTX variables
 *
 * Revision 1.5  2017-08-08 23:50:47+05:30  Cprogrammer
 * openssl 1.1.0 port
 *
 * Revision 1.4  2015-12-15 16:03:09+05:30  Cprogrammer
 * use time_t for ExpireTime
 *
 * Revision 1.3  2011-06-04 09:37:25+05:30  Cprogrammer
 * added AllowUnsignedFromHeaders
 *
 * Revision 1.2  2009-03-26 15:12:15+05:30  Cprogrammer
 * changes for ADSP
 *
 * Revision 1.1  2009-03-21 08:50:22+05:30  Cprogrammer
 * Initial revision
 */
