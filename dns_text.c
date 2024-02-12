#include "dns_text.h"
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <string.h>
#include "stralloc.h"
#include "str.h"
#include "dns.h"
#include "error.h"
#include "byte.h"
#include "alloc.h"
#include <openssl/evp.h>

#define MAX_EDNS_RESPONSE_SIZE 65536

char           *
dk_strdup(const char *s)
{
  char           *new = OPENSSL_malloc(str_len((char *) s) + 1);
  if (new != 0)
    str_copy(new, (char *) s);
  return new;
}

static unsigned short getshort(c) unsigned char *c;
{ unsigned short u; u = c[0]; return (u << 8) + c[1]; }

static struct
{
  unsigned char  *buf;
} response;
static int      responsebuflen = 0;
static int      responselen;
static unsigned char *responseend;
static unsigned char *responsepos;
static u_long   saveresoptions;
static int      (*lookup) () = res_query;
static int      numanswers;
static char     name[MAXDNAME];
static stralloc glue = {0};
static stralloc txt = { 0 };
static stralloc result = { 0 };
static stralloc sa = {0};

static int resolve(stralloc *domain,int type)
{
 int n;
 int i;

 errno = 0;
 if (!stralloc_copy(&glue,domain)) return DNS_MEM;
 if (!stralloc_0(&glue)) return DNS_MEM;
 if (!responsebuflen) {
  if ((response.buf = malloc(PACKETSZ+1)))
   responsebuflen = PACKETSZ+1;
  else return DNS_MEM;
 }

 responselen = lookup(glue.s,C_IN,type,response.buf,responsebuflen);
 if ((responselen >= responsebuflen) ||
     (responselen > 0 && (((HEADER *)response.buf)->tc)))
  {
   if (responsebuflen < MAX_EDNS_RESPONSE_SIZE) {
    unsigned char *newbuf = realloc(response.buf, MAX_EDNS_RESPONSE_SIZE);
    if (newbuf) {
     response.buf = newbuf;
     responsebuflen = MAX_EDNS_RESPONSE_SIZE;
    }
    else return DNS_MEM;
    saveresoptions = _res.options;
    _res.options |= RES_USEVC;
    responselen = lookup(glue.s,C_IN,type,response.buf,responsebuflen);
    _res.options = saveresoptions;
   }
  }
 if (responselen <= 0)
  {
   if (errno == ECONNREFUSED) return DNS_SOFT;
   if (h_errno == TRY_AGAIN) return DNS_SOFT;
   return DNS_HARD;
  }
 responseend = response.buf + responselen;
 responsepos = response.buf + sizeof(HEADER);
 n = ntohs(((HEADER *)response.buf)->qdcount);
 while (n-- > 0)
  {
   i = dn_expand(response.buf,responseend,responsepos,name,MAXDNAME);
   if (i < 0) return DNS_SOFT;
   responsepos += i;
   i = responseend - responsepos;
   if (i < QFIXEDSZ) return DNS_SOFT;
   responsepos += QFIXEDSZ;
  }
 numanswers = ntohs(((HEADER *)response.buf)->ancount);
 return 0;
}

static int findtxt(int wanttype)
{
 unsigned short rrtype;
 unsigned short rrdlen;
 int i;

 if (numanswers <= 0) return 2;
 --numanswers;
 if (responsepos == responseend) return DNS_SOFT;

 i = dn_expand(response.buf,responseend,responsepos,name,MAXDNAME);
 if (i < 0) return DNS_SOFT;
 responsepos += i;

 i = responseend - responsepos;
 if (i < 4 + 3 * 2) return DNS_SOFT;

 rrtype = getshort(responsepos);
 rrdlen = getshort(responsepos + 8);
 responsepos += 10;

 if (rrtype == wanttype)
  {
   unsigned short txtpos;
   unsigned char txtlen;

   txt.len = 0;
   for (txtpos = 0;txtpos < rrdlen;txtpos += txtlen)
    {
     txtlen = responsepos[txtpos++];
     if (txtlen > rrdlen-txtpos) txtlen = rrdlen-txtpos;
     if (!stralloc_catb(&txt,&responsepos[txtpos],txtlen)) return DNS_MEM;
    }

   responsepos += rrdlen;
   return 1;
 }

 responsepos += rrdlen;
 return 0;
}

static int
dns_txtplus(char *domain)
{
  int             r;

  if (!stralloc_copys(&sa, domain)) return DNS_MEM;
  switch (resolve(&sa, T_TXT)) {
  case DNS_MEM:
    return DNS_MEM;
  case DNS_SOFT:
    return DNS_SOFT;
  case DNS_HARD:
    return DNS_HARD;
  }
  while ((r = findtxt(T_TXT)) != 2) {
    if (r == DNS_SOFT)
      return DNS_SOFT;
    if (r == 1) {
      if (!stralloc_cat(&result, &txt))
        return DNS_MEM;
    }
  }
  if (!stralloc_0(&result))
    return DNS_MEM;
  if (result.len)
    return (0);
  return DNS_HARD;
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
  int             r;
  
  switch (r = dns_txtplus(dn))
  {
  case DNS_MEM:
  case DNS_SOFT:
    return dk_strdup("e=temp;");
  case DNS_HARD:
    return dk_strdup("e=perm;");
  }
  return dk_strdup(result.s);
}

int
DNSGetTXT(const char *domain, char *buffer, int maxlen)
{
  char           *results;
  int             len;

  results = dns_text((char *) domain);
  if (!strcmp(results, "e=perm;")) {
    free(results);
    return DNSRESP_PERM_FAIL;
  } else
  if (!strcmp(results, "e=temp;")) {
    free(results);
    return DNSRESP_TEMP_FAIL;
  }
  if ((len = strlen(results)) > maxlen - 1) {
    free(results);
    return DNSRESP_DOMAIN_NAME_TOO_LONG;
  }
  byte_copy(buffer, len, results);
  buffer[len] = 0;
  free(results);
  return DNSRESP_SUCCESS;
}
// vim: shiftwidth=2:tabstop=4:softtabstop=4
