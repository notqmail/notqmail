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

#include "dkim.h"
#include "dkimbase.h"
#include <string.h>
#include <algorithm>

CDKIMBase::CDKIMBase()
{
	m_From = NULL;
	m_Sender = NULL;
	m_hTag = NULL;
	m_hTagSize = 0;
	m_hTagPos = 0;
	m_Line = NULL;
	m_LineSize = 0;
	m_LinePos = 0;
	m_InHeaders = true;
}

CDKIMBase::~CDKIMBase()
{
	Free(m_Line);
	Free(m_From);
	Free(m_Sender);
	Free(m_hTag);
}

int
CDKIMBase::Init(void)
{
	return DKIM_SUCCESS;
}

/*
 * Alloc - allocate buffer
 */
int CDKIMBase::Alloc(char *&szBuffer, int nRequiredSize)
{
	szBuffer = new char[nRequiredSize];

	return (szBuffer == NULL) ? DKIM_OUT_OF_MEMORY : DKIM_SUCCESS;
}


/*
 * ReAlloc - extend buffer if necessary, leaving room for future expansion
 */
int CDKIMBase::ReAlloc(char *&szBuffer, int &nBufferSize, int nRequiredSize)
{
	if (nRequiredSize > nBufferSize) {
		char           *
			newp;
		int
			nNewSize = nRequiredSize + BUFFER_ALLOC_INCREMENT;

		if (Alloc(newp, nNewSize) == DKIM_SUCCESS) {
			if (szBuffer != NULL && nBufferSize > 0) {
				memcpy(newp, szBuffer, nBufferSize);
				delete[]szBuffer;
			}
			szBuffer = newp;
			nBufferSize = nNewSize;
		} else {
			return DKIM_OUT_OF_MEMORY;	/* memory alloc error! */
		}
	}
	return DKIM_SUCCESS;
}

/*
 * Process - split buffers into lines without any CRs or LFs at the end.
 */
void CDKIMBase::Free(char *szBuffer)
{
	if (szBuffer)
		delete[]szBuffer;
}

/*
 * Process - split buffers into lines without any CRs or LFs at the end.
 */
int CDKIMBase::Process(char *szBuffer, int nBufLength, bool bEOF)
{
	char           *p = szBuffer;
	char           *e = szBuffer + nBufLength;

	while (p < e) {
		if (*p != '\n' && *p != '\r') {
			if (m_LinePos >= m_LineSize) {
				int nRet = ReAlloc(m_Line, m_LineSize, m_LinePos + 1);
				if (nRet != DKIM_SUCCESS)
							/*
							 * How to distinguish between
							 * DKIM_FINISHED_BODY & DKIM_OUT_OF_MEMORY
							 */
					return nRet;
			}
			m_Line[m_LinePos++] = *p;
		} else {
			if (*p == '\r' && p + 1 < e && *(p + 1) == '\n')
				p++;
			if (m_InHeaders) {
				/* process header line */
				if (m_LinePos == 0) {
					m_InHeaders = false;
					int Result = ProcessHeaders();
					if (Result != DKIM_SUCCESS)
						return Result;
				} else {
					/* append the header to the headers list */
					if (m_Line[0] != ' ' && m_Line[0] != '\t')
						HeaderList.push_back(string(m_Line, m_LinePos));
					else {
						if (!HeaderList.empty())
							HeaderList.back().append("\r\n", 2).append(m_Line, m_LinePos);
						else {
							/* no header to append to... */
						}
					}
				}
			} else {
				/* process body line */
				int Result = ProcessBody(m_Line, m_LinePos, bEOF);
				if (Result != DKIM_SUCCESS && Result != DKIM_FINISHED_BODY) {
					m_LinePos = 0;
					return Result;
				}
			}
			m_LinePos = 0;
		}
		p++;
	}
	return DKIM_SUCCESS;
}


/*
 * ProcessFinal - process leftovers if stopping before the body or mid-line
 */
int CDKIMBase::ProcessFinal(void)
{
	if (m_LinePos > 0)
		Process((char *) "\r\n", 2, true);
	if (m_InHeaders) {
		m_InHeaders = false;
		ProcessHeaders();
		ProcessBody((char *) "", 0, true);
	}
	return DKIM_SUCCESS;
}


/*
 * ProcessHeaders - process the headers (to be implemented by derived class)
 */
int CDKIMBase::ProcessHeaders()
{
	return DKIM_SUCCESS;
}


/*
 * ProcessBody - process body line (to be implemented by derived class)
 */
int CDKIMBase::ProcessBody(char *szBuffer, int nBufLength, bool bEOF)
{
	return DKIM_SUCCESS;
}


/*
 * RemoveSWSP - remove streaming white space from buffer/string inline
 */
struct isswsp {
	bool
	operator() (char ch) {
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
	}
};

void CDKIMBase::RemoveSWSP(char *szBuffer)
{
	*remove_if(szBuffer, szBuffer + strlen(szBuffer), isswsp()) = '\0';
}

void CDKIMBase::RemoveSWSP(char *pBuffer, int &nBufLength)
{
	nBufLength = remove_if(pBuffer, pBuffer + nBufLength, isswsp()) - pBuffer;
}

void CDKIMBase::RemoveSWSP(string &sBuffer)
{
	sBuffer.erase(remove_if(sBuffer.begin(), sBuffer.end(), isswsp()), sBuffer.end());
}


/*
 * CompressSWSP - compress streaming white space into single spaces from buffer/string inline
 */
void CDKIMBase::CompressSWSP(char *pBuffer, int &nBufLength)
{
	char           *pSrc = pBuffer;
	char           *pDst = pBuffer;
	char           *pEnd = pBuffer + nBufLength;

	while (pSrc != pEnd) {
		if (isswsp()(*pSrc)) {
			do {
				++pSrc;
			} while (pSrc != pEnd && isswsp()(*pSrc));
			if (pSrc == pEnd)
				break;
			*pDst++ = ' ';
		}
		*pDst++ = *pSrc++;
	}
	nBufLength = pDst - pBuffer;
}

void CDKIMBase::CompressSWSP(string &sBuffer)
{
	string::iterator iSrc = sBuffer.begin();
	string::iterator iDst = sBuffer.begin();
	string::iterator iEnd = sBuffer.end();

	while (iSrc != iEnd) {
		if (isswsp()(*iSrc)) {
			do {
				++iSrc;
			} while (iSrc != iEnd && isswsp()(*iSrc));

			if (iSrc == iEnd)
				break;
			*iDst++ = ' ';
		}
		*iDst++ = *iSrc++;
	}
	sBuffer.erase(iDst, iEnd);
}

/*
 * RelaxHeader - relax a header field (lower case the name, remove swsp before and after :)
 *
 * modified 4/21/06 STB to remove white space before colon
 */
string CDKIMBase::RelaxHeader(const string &sHeader)
{
	string sTemp = sHeader;

	CompressSWSP(sTemp);

	int cpos = sTemp.find(':');
	if (cpos == -1) {
		/* no colon?! */
	} else {
		/* lower case the header field name */
		for (int i = 0; i < cpos; i++) {
			if (sTemp[i] >= 'A' && sTemp[i] <= 'Z')
				sTemp[i] += 'a' - 'A';
		}
		/* remove the space after the : */
		if ((unsigned int) (cpos + 1) < sTemp.length() && sTemp[cpos + 1] == ' ')
			sTemp.erase(cpos + 1, 1);
		/* remove the space before the : */
		if (cpos > 0 && sTemp[cpos - 1] == ' ')
			sTemp.erase(cpos - 1, 1);
	}
	return sTemp;
}

void
getversion_dkimbase_cpp()
{
	static char    *x = (char *) "$Id: dkimbase.cpp,v 1.6 2023-02-04 07:55:12+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: dkimbase.cpp,v $
 * Revision 1.6  2023-02-04 07:55:12+05:30  Cprogrammer
 * converted comments to traditional C style comments
 *
 * Revision 1.5  2019-06-14 21:24:03+05:30  Cprogrammer
 * BUG - honor body length tag in verification
 *
 * Revision 1.4  2017-09-05 10:58:26+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.3  2009-03-26 15:10:32+05:30  Cprogrammer
 * fixed indentation
 *
 * Revision 1.2  2009-03-25 08:37:27+05:30  Cprogrammer
 * fixed indentation
 *
 * Revision 1.1  2009-03-21 08:43:08+05:30  Cprogrammer
 * Initial revision
 */
