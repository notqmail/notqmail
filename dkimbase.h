/*
 * $Log: dkimbase.h,v $
 * Revision 1.1  2009-03-21 08:50:18+05:30  Cprogrammer
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

#ifndef DKIMBASE_H
#define DKIMBASE_H

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define BUFFER_ALLOC_INCREMENT	256

#include <string>
#include <list>

using namespace std;
class           CDKIMBase {
public:

	CDKIMBase();
	~CDKIMBase();

	int             Init(void);
	int             Process(char *szBuffer, int nBufLength, bool bEOF);
	int             ProcessFinal(void);
	int             Alloc(char *&szBuffer, int nRequiredSize);
	int             ReAlloc(char *&szBuffer, int &nBufferLength, int nRequiredSize);
	void            Free(char *szBuffer);
	static void     RemoveSWSP(char *szBuffer);
	static void     RemoveSWSP(char *pBuffer, int &nBufLength);
	static void     RemoveSWSP(string & sBuffer);
	static void     CompressSWSP(char *pBuffer, int &nBufLength);
	static void     CompressSWSP(string & sBuffer);
	static string   RelaxHeader(const string & sHeader);
	virtual int     ProcessHeaders(void);
	virtual int     ProcessBody(char *szBuffer, int nBufLength, bool bEOF);

protected:
	char           *m_From;
	char           *m_Sender;
	char           *m_hTag;
	int             m_hTagSize;
	int             m_hTagPos;
	char           *m_Line;
	int             m_LineSize;
	int             m_LinePos;
	bool            m_InHeaders;
	                list < string > HeaderList;
};
#endif	/*- DKIMBASE_H */
