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
#ifndef _DNS_TXT_H
#define _DNS_TXT_H

#define MAX_DOMAIN			254

#define DNSRESP_SUCCESS					0	// DNS lookup returned sought after records
#define DNSRESP_TEMP_FAIL				1	// No response from DNS server
#define DNSRESP_PERM_FAIL				2	// DNS server returned error or no records
#define DNSRESP_DOMAIN_NAME_TOO_LONG	3	// Domain name too long
#define DNSRESP_NXDOMAIN				4	// DNS server returned Name Error
#define DNSRESP_EMPTY					5	// DNS server returned successful response but no records

#define DNS_SOFT -1
#define DNS_HARD -2
#define DNS_MEM  -3

extern char *dns_text(char *szFQDN);
extern int DNSGetTXT(const char *domain, char *buffer, int maxlen);
#endif
