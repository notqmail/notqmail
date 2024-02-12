/*
 * $Log: dkimfuncs.cpp,v $
 * Revision 1.5  2021-08-28 21:41:03+05:30  Cprogrammer
 * added function to replace selector
 *
 * Revision 1.4  2011-06-04 10:06:33+05:30  Cprogrammer
 * unified error strings for signing & verification
 *
 * Revision 1.3  2009-04-15 20:45:29+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.2  2009-03-26 15:11:12+05:30  Cprogrammer
 * added GetDomain(), ADSP
 *
 * Revision 1.1  2009-03-21 08:43:10+05:30  Cprogrammer
 * Initial revision
 *
 *
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "dkim.h"
#include "dkimsign.h"
#include "dkimverify.h"
#include <string.h>

#define DKIMID ('D' | 'K'<<8 | 'I'<<16 | 'M'<<24)

static void
InitContext(DKIMContext *pContext, bool bSign, void *pObject)
{
	pContext->reserved1 = DKIMID;
	pContext->reserved2 = bSign ? 1 : 0;
	pContext->reserved3 = pObject;
}

static void    *
ValidateContext(DKIMContext *pContext, bool bSign)
{
	if (pContext->reserved1 != DKIMID)
		return NULL;
	if (pContext->reserved2 != (unsigned int) (bSign ? 1 : 0))
		return NULL;
	return pContext->reserved3;
}

int             DKIM_CALL
DKIMSignInit(DKIMContext *pSignContext, DKIMSignOptions *pOptions)
{
	int             nRet = DKIM_OUT_OF_MEMORY;
	CDKIMSign      *pSign = new CDKIMSign;

	if (pSign) {
		nRet = pSign->Init(pOptions);
		if (nRet != DKIM_SUCCESS)
			delete          pSign;
	}
	if (nRet == DKIM_SUCCESS)
		InitContext(pSignContext, true, pSign);
	return nRet;
}

int             DKIM_CALL
DKIMSignReplaceSelector(DKIMContext *pSignContext, DKIMSignOptions *pOptions)
{
	CDKIMSign      *pSign = (CDKIMSign *) ValidateContext(pSignContext, true);

	if (pSign)
		pSign->ReplaceSelector(pOptions);
	return DKIM_INVALID_CONTEXT;
}

int             DKIM_CALL
DKIMSignReplaceHash(DKIMContext *pSignContext, DKIMSignOptions *pOptions)
{
	CDKIMSign      *pSign = (CDKIMSign *) ValidateContext(pSignContext, true);

	if (pSign)
		pSign->ReplaceHash(pOptions);
	return DKIM_INVALID_CONTEXT;
}

int             DKIM_CALL
DKIMSignProcess(DKIMContext *pSignContext, char *szBuffer, int nBufLength)
{
	CDKIMSign      *pSign = (CDKIMSign *) ValidateContext(pSignContext, true);
	if (pSign)
		return pSign->Process(szBuffer, nBufLength, false);
	return DKIM_INVALID_CONTEXT;
}

int             DKIM_CALL
DKIMSignGetSig(DKIMContext *pSignContext, char *szPrivKey, char *szSignature, int nSigLength)
{
	CDKIMSign      *pSign = (CDKIMSign *) ValidateContext(pSignContext, true);
	if (pSign)
		return pSign->GetSig(szPrivKey, szSignature, nSigLength);
	return DKIM_INVALID_CONTEXT;
}

int             DKIM_CALL
DKIMSignGetSig2(DKIMContext *pSignContext, char *szPrivKey, char **pszSignature)
{
	CDKIMSign      *pSign = (CDKIMSign *) ValidateContext(pSignContext, true);
	if (pSign)
		return pSign->GetSig2(szPrivKey, pszSignature);
	return DKIM_INVALID_CONTEXT;
}

char           *DKIM_CALL
DKIMSignGetDomain(DKIMContext *pSignContext)
{
	CDKIMSign      *pSign = (CDKIMSign *) ValidateContext(pSignContext, true);
	if (pSign)
		return pSign->GetDomain();
	return ((char *) 0);
}

void            DKIM_CALL
DKIMSignFree(DKIMContext *pSignContext)
{
	CDKIMSign      *pSign = (CDKIMSign *) ValidateContext(pSignContext, true);
	if (pSign) {
		delete          pSign;
		pSignContext->reserved3 = NULL;
	}
}

int             DKIM_CALL
DKIMVerifyInit(DKIMContext *pVerifyContext, DKIMVerifyOptions *pOptions)
{
	int             nRet = DKIM_OUT_OF_MEMORY;
	CDKIMVerify    *pVerify = new CDKIMVerify;
	if (pVerify) {
		nRet = pVerify->Init(pOptions);
		if (nRet != DKIM_SUCCESS)
			delete          pVerify;
	}
	if (nRet == DKIM_SUCCESS)
		InitContext(pVerifyContext, false, pVerify);
	return nRet;
}

int             DKIM_CALL
DKIMVerifyProcess(DKIMContext *pVerifyContext, char *szBuffer, int nBufLength)
{
	CDKIMVerify    *pVerify = (CDKIMVerify *) ValidateContext(pVerifyContext, false);
	if (pVerify)
		return pVerify->Process(szBuffer, nBufLength, false);
	return DKIM_INVALID_CONTEXT;
}

int             DKIM_CALL
DKIMVerifyResults( DKIMContext *pVerifyContext , int *sCount, int *sSize)
{
	CDKIMVerify    *pVerify = (CDKIMVerify *) ValidateContext(pVerifyContext, false);
	if (pVerify)
		return pVerify->GetResults(sCount, sSize);
	return DKIM_INVALID_CONTEXT;
}

int             DKIM_CALL
DKIMVerifyGetDetails(DKIMContext *pVerifyContext, int *nSigCount, DKIMVerifyDetails **pDetails, char *szPractices)
{
	szPractices[0] = '\0';
	CDKIMVerify    *pVerify = (CDKIMVerify *) ValidateContext(pVerifyContext, false);
	if (pVerify) {
		strcpy(szPractices, pVerify->GetPractices());
		return pVerify->GetDetails(nSigCount, pDetails);
	}
	return DKIM_INVALID_CONTEXT;
}

void            DKIM_CALL
DKIMVerifyFree(DKIMContext *pVerifyContext)
{
	CDKIMVerify    *pVerify = (CDKIMVerify *) ValidateContext(pVerifyContext, false);
	if (pVerify) {
		delete          pVerify;
		pVerifyContext->reserved3 = NULL;
	}
}

char           *DKIM_CALL
DKIMVerifyGetDomain(DKIMContext *pVerifyContext)
{
	CDKIMVerify    *pVerify = (CDKIMVerify *) ValidateContext(pVerifyContext, false);
	if (pVerify)
		return pVerify->GetDomain();
	return ((char *) 0);
}

static char    *DKIMErrorStrings[-1 - DKIM_MAX_ERROR] = {
	(char *) "DKIM_FAIL",
	(char *) "DKIM_BAD_SYNTAX",
	(char *) "DKIM_SIGNATURE_BAD",
	(char *) "DKIM_SIGNATURE_BAD_BUT_TESTING",
	(char *) "DKIM_SIGNATURE_EXPIRED",
	(char *) "DKIM_SELECTOR_INVALID",
	(char *) "DKIM_SELECTOR_GRANULARITY_MISMATCH",
	(char *) "DKIM_SELECTOR_KEY_REVOKED",
	(char *) "DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG",
	(char *) "DKIM_SELECTOR_DNS_TEMP_FAILURE",
	(char *) "DKIM_SELECTOR_DNS_PERM_FAILURE",
	(char *) "DKIM_SELECTOR_PUBLIC_KEY_INVALID",
	(char *) "DKIM_NO_SIGNATURES",
	(char *) "DKIM_NO_VALID_SIGNATURES",
	(char *) "DKIM_BODY_HASH_MISMATCH",
	(char *) "DKIM_SELECTOR_ALGORITHM_MISMATCH",
	(char *) "DKIM_STAT_INCOMPAT",
	(char *) "DKIM_UNSIGNED_FROM",
	(char *) "DKIM_OUT_OF_MEMORY",
	(char *) "DKIM_INVALID_CONTEXT",
	(char *) "DKIM_NO_SENDER",
	(char *) "DKIM_BAD_PRIVATE_KEY",
	(char *) "DKIM_BUFFER_TOO_SMALL"
};

char           *DKIM_CALL
DKIMGetErrorString(int ErrorCode)
{
	if (ErrorCode >= 0 || ErrorCode <= DKIM_MAX_ERROR)
		return (char *) "Unknown";

	else
		return DKIMErrorStrings[-1 - ErrorCode];
}

void
getversion_dkimfuncs_cpp()
{
	static char    *x = (char *) "$Id: dkimfuncs.cpp,v 1.5 2021-08-28 21:41:03+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
