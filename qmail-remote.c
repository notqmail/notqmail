#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sig.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "scan.h"
#include "case.h"
#include "error.h"
#include "auto_qmail.h"
#include "control.h"
#include "dns.h"
#include "alloc.h"
#include "quote.h"
#include "ip.h"
#include "ipalloc.h"
#include "ipme.h"
#include "gen_alloc.h"
#include "gen_allocdefs.h"
#include "str.h"
#include "now.h"
#include "exit.h"
#include "constmap.h"
#include "tcpto.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

#define HUGESMTPTEXT 5000

#define PORT_SMTP 25 /* silly rabbit, /etc/services is for users */
unsigned long port = PORT_SMTP;

GEN_ALLOC_typedef(saa,stralloc,sa,len,a)
GEN_ALLOC_readyplus(saa,stralloc,sa,len,a,i,n,x,10,saa_readyplus)
static stralloc sauninit = {0};

stralloc helohost = {0};
stralloc routes = {0};
struct constmap maproutes;
stralloc host = {0};
stralloc sender = {0};

saa reciplist = {0};

struct ip_address partner;

void out(s) char *s; { if (substdio_puts(subfdout,s) == -1) _exit(0); }
void zero() { if (substdio_put(subfdout,"\0",1) == -1) _exit(0); }
void zerodie() { zero(); substdio_flush(subfdout); _exit(0); }

void outsafe(sa) stralloc *sa; { int i; char ch;
for (i = 0;i < sa->len;++i) {
ch = sa->s[i]; if (ch < 33) ch = '?'; if (ch > 126) ch = '?';
if (substdio_put(subfdout,&ch,1) == -1) _exit(0); } }

void temp_nomem() { out("ZOut of memory. (#4.3.0)\n"); zerodie(); }
void temp_oserr() { out("Z\
System resources temporarily unavailable. (#4.3.0)\n"); zerodie(); }
void temp_noconn() { out("Z\
Sorry, I wasn't able to establish an SMTP connection. (#4.4.1)\n"); zerodie(); }
void temp_read() { out("ZUnable to read message. (#4.3.0)\n"); zerodie(); }
void temp_dnscanon() { out("Z\
CNAME lookup failed temporarily. (#4.4.3)\n"); zerodie(); }
void temp_dns() { out("Z\
Sorry, I couldn't find any host by that name. (#4.1.2)\n"); zerodie(); }
void temp_chdir() { out("Z\
Unable to switch to home directory. (#4.3.0)\n"); zerodie(); }
void temp_control() { out("Z\
Unable to read control files. (#4.3.0)\n"); zerodie(); }
void perm_partialline() { out("D\
SMTP cannot transfer messages with partial final lines. (#5.6.2)\n"); zerodie(); }
void perm_usage() { out("D\
I (qmail-remote) was invoked improperly. (#5.3.5)\n"); zerodie(); }
void perm_dns() { out("D\
Sorry, I couldn't find any host named ");
outsafe(&host);
out(". (#5.1.2)\n"); zerodie(); }
void perm_nomx() { out("D\
Sorry, I couldn't find a mail exchanger or IP address. (#5.4.4)\n");
zerodie(); }
void perm_ambigmx() { out("D\
Sorry. Although I'm listed as a best-preference MX or A for that host,\n\
it isn't in my control/locals file, so I don't treat it as local. (#5.4.6)\n");
zerodie(); }

int timeout = 1200;
int timeoutconnect = 60;

void getcontrols()
{
 int r;
 if (control_init() == -1)
  { if (errno == error_nomem) temp_nomem(); temp_control(); }

 if (control_readint(&timeout,"control/timeoutremote") == -1)
  { if (errno == error_nomem) temp_nomem(); temp_control(); }
 if (control_readint(&timeoutconnect,"control/timeoutconnect") == -1)
  { if (errno == error_nomem) temp_nomem(); temp_control(); }

 r = control_rldef(&helohost,"control/helohost",1,(char *) 0);
 if (r == -1) if (errno == error_nomem) temp_nomem();
 if (r != 1) temp_control();

 switch(control_readfile(&routes,"control/smtproutes",0))
  {
   case -1:
     if (errno == error_nomem) temp_nomem(); temp_control();
   case 0:
     if (!constmap_init(&maproutes,"",0,1)) temp_nomem(); break;
   case 1:
     if (!constmap_init(&maproutes,routes.s,routes.len,1)) temp_nomem(); break;
  }
}

char smtptobuf[1024];
char smtpfrombuf[128];
stralloc smtpline = {0};
stralloc smtptext = {0};

void outsmtptext()
{
 int i; 
 if (smtptext.s) if (smtptext.len) if (smtptext.len < HUGESMTPTEXT)
  {
   if (substdio_puts(subfdout,"Remote host said: ") == -1) _exit(0);
   for (i = 0;i < smtptext.len;++i)
     if (!smtptext.s[i]) smtptext.s[i] = '?';
   if (substdio_put(subfdout,smtptext.s,smtptext.len) == -1) _exit(0);
   smtptext.len = 0;
  }
}

unsigned long smtpcode(ss)
substdio *ss;
{
 int match;
 unsigned long code;

 if (!stralloc_copys(&smtptext,"")) return 421;
 do
  {
   if (getln(ss,&smtpline,&match,'\n') != 0) return 421;
   if (!match) return 421;
   if ((smtpline.len >= 2) && (smtpline.s[smtpline.len - 2] == '\r'))
    {
     smtpline.s[smtpline.len - 2] = '\n';
     --smtpline.len;
    }
   if (!stralloc_cat(&smtptext,&smtpline)) return 421;
   if (scan_nbblong(smtpline.s,smtpline.len,10,0,&code) != 3) return 421;
   if (smtpline.len == 3) return code;
  }
 while (smtpline.s[3] == '-');

 return code;
}

void outhost()
{
 char x[IPFMT];

 x[ip_fmt(x,&partner)] = 0;
 out(x);
}

void writeerr()
{
 out("ZConnected to "); outhost();
 out(" but communications failed. (#4.4.2)\n");
 zerodie();
}

void quit(ssto,ssfrom)
substdio *ssto;
substdio *ssfrom;
{
 outsmtptext();
 if (substdio_putsflush(ssto,"QUIT\r\n") != -1)
   smtpcode(ssfrom); /* protocol design stupidity */
 zerodie();
}

stralloc dataline = {0};

void blast(ssto,ssfrom)
substdio *ssto;
substdio *ssfrom;
{
 int match;

 for (;;)
  {
   if (getln(ssfrom,&dataline,&match,'\n') != 0) temp_read();
   if (!match && !dataline.len) break;
   if (!match) perm_partialline();
   --dataline.len;
   if (dataline.len && (dataline.s[0] == '.'))
     if (substdio_put(ssto,".",1) == -1) writeerr();
   if (substdio_put(ssto,dataline.s,dataline.len) == -1) writeerr();
   if (substdio_put(ssto,"\r\n",2) == -1) writeerr();
  }
 if (substdio_put(ssto,".\r\n",3) == -1) writeerr();
 if (substdio_flush(ssto) == -1) writeerr();
}

stralloc recip = {0};

void smtp(fd)
int fd;
{
 substdio ssto;
 substdio ssfrom;
 unsigned long code;
 int flaganyrecipok;
 int i;

 substdio_fdbuf(&ssto,timeoutwrite,TIMEOUTWRITE(timeout,fd),smtptobuf,sizeof(smtptobuf));
 substdio_fdbuf(&ssfrom,timeoutread,TIMEOUTREAD(timeout,fd),smtpfrombuf,sizeof(smtpfrombuf));

 if (smtpcode(&ssfrom) != 220)
  {
   out("ZConnected to "); outhost(); out(" but greeting failed.\n");
   quit(&ssto,&ssfrom);
  }

 if (substdio_puts(&ssto,"HELO ") == -1) writeerr();
 if (substdio_put(&ssto,helohost.s,helohost.len) == -1) writeerr();
 if (substdio_puts(&ssto,"\r\n") == -1) writeerr();
 if (substdio_flush(&ssto) == -1) writeerr();

 if (smtpcode(&ssfrom) != 250)
  {
   out("ZConnected to "); outhost(); out(" but my name was rejected.\n");
   quit(&ssto,&ssfrom);
  }

 if (substdio_puts(&ssto,"MAIL FROM:<") == -1) writeerr();
 if (substdio_put(&ssto,sender.s,sender.len) == -1) writeerr();
 if (substdio_puts(&ssto,">\r\n") == -1) writeerr();
 if (substdio_flush(&ssto) == -1) writeerr();

 code = smtpcode(&ssfrom);
 if (code >= 500)
  {
   out("DConnected to "); outhost(); out(" but sender was rejected.\n");
   quit(&ssto,&ssfrom);
  }
 if (code >= 400)
  {
   out("ZConnected to "); outhost(); out(" but sender was rejected.\n");
   quit(&ssto,&ssfrom);
  }

 flaganyrecipok = 0;
 for (i = 0;i < reciplist.len;++i)
  {
   if (substdio_puts(&ssto,"RCPT TO:<") == -1) writeerr();
   if (substdio_put(&ssto,reciplist.sa[i].s,reciplist.sa[i].len) == -1) writeerr();
   if (substdio_puts(&ssto,">\r\n") == -1) writeerr();
   if (substdio_flush(&ssto) == -1) writeerr();

   code = smtpcode(&ssfrom);
   if (code == 421)
    {
     out("ZConnected to "); outhost(); out(" but connection died.\n");
     quit(&ssto,&ssfrom);
    }
   if (code >= 500)
    {
     out("h"); outhost(); out(" does not like recipient.\n");
     outsmtptext(); zero();
    }
   else if (code >= 400)
    {
     out("s"); outhost(); out(" does not like recipient.\n");
     outsmtptext(); zero();
    }
   else
    {
     out("r"); zero();
     flaganyrecipok = 1;
    }
  }

 if (!flaganyrecipok)
  {
   out("DGiving up.\n");
   quit(&ssto,&ssfrom);
  }

 if (substdio_putsflush(&ssto,"DATA\r\n") == -1) writeerr();

 code = smtpcode(&ssfrom);
 if (code == 421)
  {
   out("ZConnected to "); outhost(); out(" but connection died.\n");
   quit(&ssto,&ssfrom);
  }
 if (code >= 500)
  {
   out("D"); outhost(); out(" failed on DATA command.\n");
   quit(&ssto,&ssfrom);
  }
 if (code >= 400)
  {
   out("Z"); outhost(); out(" failed on DATA command.\n");
   quit(&ssto,&ssfrom);
  }

 blast(&ssto,subfdin);

 code = smtpcode(&ssfrom);
 if (code == 421)
  {
   out("ZConnected to "); outhost(); out(" but connection died. Possible duplicate!\n");
   quit(&ssto,&ssfrom);
  }
 if (code >= 500)
  {
   out("D"); outhost(); out(" failed after I sent the message.\n");
   quit(&ssto,&ssfrom);
  }
 if (code >= 400)
  {
   out("Z"); outhost(); out(" failed after I sent the message.\n");
   quit(&ssto,&ssfrom);
  }

 out("K"); outhost(); out(" accepted message.\n");
 quit(&ssto,&ssfrom);
}

stralloc canonhost = {0};
stralloc canonbox = {0};

void addrmangle(saout,s,flagalias,flagcname)
stralloc *saout; /* host has to be canonical, box has to be quoted */
char *s;
int *flagalias;
int flagcname;
{
 int j;

 *flagalias = flagcname;

 j = str_rchr(s,'@');
 if (!s[j])
  {
   if (!stralloc_copys(saout,s)) temp_nomem();
   return;
  }
 if (!stralloc_copys(&canonbox,s)) temp_nomem();
 canonbox.len = j;
 if (!quote(saout,&canonbox)) temp_nomem();
 if (!stralloc_cats(saout,"@")) temp_nomem();

 if (!stralloc_copys(&canonhost,s + j + 1)) temp_nomem();
 if (flagcname)
   switch(dns_cname(&canonhost))
    {
     case 0: *flagalias = 0; break;
     case DNS_MEM: temp_nomem();
     case DNS_SOFT: temp_dnscanon();
     case DNS_HARD: ; /* alias loop, not our problem */
    }

 if (!stralloc_cat(saout,&canonhost)) temp_nomem();
}

void main(argc,argv)
int argc;
char **argv;
{
 static ipalloc ip = {0};
 int i;
 unsigned long random;
 char **recips;
 unsigned long prefme;
 int flagallaliases;
 int flagalias;
 char *relayhost;

 sig_pipeignore();
 if (argc < 4) perm_usage();
 if (chdir(auto_qmail) == -1) temp_chdir();
 getcontrols();


 if (!stralloc_copys(&host,argv[1])) temp_nomem();

 relayhost = 0;
 for (i = 0;i <= host.len;++i)
   if ((i == 0) || (i == host.len) || (host.s[i] == '.'))
     if (relayhost = constmap(&maproutes,host.s + i,host.len - i))
       break;
 if (relayhost && !*relayhost) relayhost = 0;

 if (relayhost)
  {
   i = str_chr(relayhost,':');
   if (relayhost[i])
    {
     scan_ulong(relayhost + i + 1,&port);
     relayhost[i] = 0;
    }
   if (!stralloc_copys(&host,relayhost)) temp_nomem();
  }


 addrmangle(&sender,argv[2],&flagalias,!relayhost);

 if (!saa_readyplus(&reciplist,0)) temp_nomem();
 if (ipme_init() != 1) temp_oserr();

 flagallaliases = 1;
 recips = argv + 3;
 while (*recips)
  {
   if (!saa_readyplus(&reciplist,1)) temp_nomem();
   reciplist.sa[reciplist.len] = sauninit;
   addrmangle(reciplist.sa + reciplist.len,*recips,&flagalias,!relayhost);
   if (!flagalias) flagallaliases = 0;
   ++reciplist.len;
   ++recips;
  }


 random = now() + (getpid() << 16);
 switch (relayhost ? dns_ip(&ip,&host) : dns_mxip(&ip,&host,random))
  {
   case DNS_MEM: temp_nomem();
   case DNS_SOFT: temp_dns();
   case DNS_HARD: perm_dns();
   case 1:
     if (ip.len <= 0) temp_dns();
  }

 if (ip.len <= 0) perm_nomx();

 prefme = 100000;
 for (i = 0;i < ip.len;++i)
   if (ipme_is(&ip.ix[i].ip))
     if (ip.ix[i].pref < prefme)
       prefme = ip.ix[i].pref;

 if (relayhost) prefme = 300000;
 if (flagallaliases) prefme = 500000;

 for (i = 0;i < ip.len;++i)
   if (ip.ix[i].pref < prefme)
     break;

 if (i >= ip.len)
   perm_ambigmx();

 for (i = 0;i < ip.len;++i) if (ip.ix[i].pref < prefme)
  {
   int s;

   if (tcpto(&ip.ix[i].ip)) continue;

   s = socket(AF_INET,SOCK_STREAM,0);
   if (s == -1) temp_oserr();

   if (timeoutconn(s,&ip.ix[i].ip,(unsigned int) port,timeoutconnect) == 0)
    {
     tcpto_err(&ip.ix[i].ip,0);
     partner = ip.ix[i].ip;
     smtp(s); /* does not return */
    }
   tcpto_err(&ip.ix[i].ip,errno == error_timeout);
   close(s);
  }
 
 temp_noconn();
}
