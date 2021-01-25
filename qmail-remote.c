#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sig.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "scan.h"
#include "case.h"
#include "error.h"
#include "auto_qmail.h"
#include "control.h"
#include "dns.h"
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
#include "readwrite.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "hassmtputf8.h"
#ifdef SMTPUTF8
#include <idn2.h>
#endif
#include "utf8read.h"
#include "env.h"

#define HUGESMTPTEXT 5000

#define PORT_SMTP 25 /* silly rabbit, /etc/services is for users */
unsigned long port = PORT_SMTP;

GEN_ALLOC_typedef(saa,stralloc,sa,len,a)
GEN_ALLOC_readyplus(saa,stralloc,sa,len,a,10,saa_readyplus)
static stralloc sauninit = {0};

stralloc helohost = {0};
stralloc routes = {0};
struct constmap maproutes;
stralloc host = {0};
stralloc sender = {0};

saa reciplist = {0};

struct ip_address partner;

static stralloc idnhost = { 0 };
static int smtputf8 = 0; /*- if remote has SMTPUTF8 capability */
/* set by control file control/smtputf8. zero if SMTPUTF8 not #defined */
static int enable_smtputf8; 
int flagutf8; /*- if sender, recipient headers, body have utf8 */

void out(s) char *s; { if (substdio_puts(subfdoutsmall,s) == -1) _exit(0); }
void zero() { if (substdio_put(subfdoutsmall,"\0",1) == -1) _exit(0); }
void zerodie() { zero(); substdio_flush(subfdoutsmall); _exit(0); }
void outsafe(sa) stralloc *sa; { int i; char ch;
for (i = 0;i < sa->len;++i) {
ch = sa->s[i]; if (ch < 33) ch = '?'; if (ch > 126) ch = '?';
if (substdio_put(subfdoutsmall,&ch,1) == -1) _exit(0); } }

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

void outhost()
{
  char x[IPFMT];
  if (substdio_put(subfdoutsmall,x,ip_fmt(x,&partner)) == -1) _exit(0);
}

int flagcritical = 0;

void dropped() {
  out("ZConnected to ");
  outhost();
  out(" but connection died. ");
  if (flagcritical) out("Possible duplicate! ");
  out("(#4.4.2)\n");
  zerodie();
}

int timeoutconnect = 60;
int smtpfd;
int timeout = 1200;

GEN_SAFE_TIMEOUTREAD(saferead,timeout,smtpfd,dropped())
GEN_SAFE_TIMEOUTWRITE(safewrite,timeout,smtpfd,dropped())

char inbuf[1024];
substdio ssin = SUBSTDIO_FDBUF(read,0,inbuf,sizeof(inbuf));
char smtptobuf[1024];
substdio smtpto = SUBSTDIO_FDBUF(safewrite,-1,smtptobuf,sizeof(smtptobuf));
char smtpfrombuf[128];
substdio smtpfrom = SUBSTDIO_FDBUF(saferead,-1,smtpfrombuf,sizeof(smtpfrombuf));

stralloc smtptext = {0};

static void get(unsigned char *uc)
{
  char *ch = (char *)uc;
  substdio_get(&smtpfrom,ch,1);
  if (*ch != '\r' && smtptext.len < HUGESMTPTEXT &&
      !stralloc_append(&smtptext,ch))
    temp_nomem();
}

static unsigned long get3()
{
  char            str[4];
  int             i;
  unsigned long   code;

  substdio_get(&smtpfrom,str,3);
  str[3] = 0;
  for (i = 0;i < 3;i++) {
    if (str[i] == '\r') continue;
    if (smtptext.len < HUGESMTPTEXT &&
        !stralloc_append(&smtptext,str+i))
      temp_nomem();
  }
  scan_ulong(str,&code);
  return code;
}

unsigned long smtpcode()
{
  unsigned char   ch;
  unsigned long   code;
  int             err = 0;

  if (!stralloc_copys(&smtptext,"")) temp_nomem();
  if ((code = get3()) < 200) err = 1;
  for (;;) {
    get((char *)&ch);
    if (ch != ' ' && ch != '-') err = 1;
    if (ch != '-') break;
    while (ch != '\n') get((char *)&ch);
    if (get3() != code) err = 1;
  }
  while (ch != '\n') get((char *)&ch);
  return err ? 400 : code;
}

void outsmtptext()
{
  int i; 
  if (smtptext.s) if (smtptext.len) {
    out("Remote host said: ");
    for (i = 0;i < smtptext.len;++i)
      if (!smtptext.s[i]) smtptext.s[i] = '?';
    if (substdio_put(subfdoutsmall,smtptext.s,smtptext.len) == -1) _exit(0);
    smtptext.len = 0;
  }
}

void quit(prepend,append)
char *prepend;
char *append;
{
  substdio_putsflush(&smtpto,"QUIT\r\n");
  /* waiting for remote side is just too ridiculous */
  out(prepend);
  outhost();
  out(append);
  out(".\n");
  outsmtptext();
  zerodie();
}

void blast()
{
  int r;
  char ch;

  for (;;) {
    r = substdio_get(&ssin,&ch,1);
    if (r == 0) break;
    if (r == -1) temp_read();
    if (ch == '.')
      substdio_put(&smtpto,".",1);
    while (ch != '\n') {
      if (ch == '\r') {
        r = substdio_get(&ssin, &ch, 1);
        if (r == 0)
          break;
        if (r == -1) temp_read();
        if (ch != '\n') {
          substdio_put(&smtpto, "\r\n", 2);
        } else
          break;
      }
      substdio_put(&smtpto,&ch,1);
      r = substdio_get(&ssin,&ch,1);
      if (r == 0) perm_partialline();
      if (r == -1) temp_read();
    }
    substdio_put(&smtpto,"\r\n",2);
  }
 
  flagcritical = 1;
  substdio_put(&smtpto,".\r\n",3);
  substdio_flush(&smtpto);
}

#ifdef SMTPUTF8
/*
 * this function is general purpose
 * we could use it when we add AUTH,
 * STARTTLS, SIZE extensions
 */
int get_capability(const char *capa)
{
  int i = 0, len;

  /* e.g.
   * 250-PIPELINING
   * 250-8BITMIME
   * 250-SIZE 10000000
   * 250-ETRN
   * 250-STARTTLS
   * 250-SMTPUTF8
   * 250 HELP
   */
  len = str_len(capa);
  for (i = 0; i < smtptext.len-len; ++i)
    if (case_starts(smtptext.s+i,capa)) return 1;

  return 0;
}
#endif

stralloc recip = {0};

void smtp()
{
  unsigned long code;
  int flagbother;
  int i;
 
  if (smtpcode() != 220) quit("ZConnected to "," but greeting failed");
 
#ifdef SMTPUTF8
  /*-
   * this part of the code is general purpose
   * it can be used for
   * checking other extensions like
   * AUTH, STARTTLS, SIZE, etc
   */
  substdio_puts(&smtpto,"EHLO ");
  substdio_put(&smtpto,helohost.s,helohost.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);

  if (smtpcode() != 250) { /*- do a HELO if EHLO fails */
    substdio_puts(&smtpto,"HELO ");
    substdio_put(&smtpto,helohost.s,helohost.len);
    substdio_puts(&smtpto,"\r\n");
    substdio_flush(&smtpto);

    code = smtpcode();
    if (code >= 500) quit("DConnected to "," but my name was rejected");
    if (code != 250) quit("ZConnected to "," but my name was rejected");
  } else /* EHLO succeeded. Let's check SMTPUTF8 capa */
      smtputf8 = get_capability("SMTPUTF8"); /*- did the remote server advertize SMTPUTF8 */
  if (!flagutf8)
    flagutf8 = utf8read();
  if (enable_smtputf8 && flagutf8 && !smtputf8)
    quit("DConnected to "," but server does not support internationalized email addresses");
#else
  substdio_puts(&smtpto,"HELO ");
  substdio_put(&smtpto,helohost.s,helohost.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() != 250) quit("ZConnected to "," but my name was rejected");
#endif
 
  substdio_puts(&smtpto,"MAIL FROM:<");
  substdio_put(&smtpto,sender.s,sender.len);
  if (enable_smtputf8 && flagutf8)
    substdio_puts(&smtpto,"> SMTPUTF8\r\n");
  else
    substdio_puts(&smtpto,">\r\n");
  substdio_flush(&smtpto);
  code = smtpcode();
  if (code >= 500) quit("DConnected to "," but sender was rejected");
  if (code >= 400) quit("ZConnected to "," but sender was rejected");
 
  flagbother = 0;
  for (i = 0;i < reciplist.len;++i) {
    substdio_puts(&smtpto,"RCPT TO:<");
    substdio_put(&smtpto,reciplist.sa[i].s,reciplist.sa[i].len);
    substdio_puts(&smtpto,">\r\n");
    substdio_flush(&smtpto);
    code = smtpcode();
    if (code >= 500) {
      out("h"); outhost(); out(" does not like recipient.\n");
      outsmtptext(); zero();
    }
    else if (code >= 400) {
      out("s"); outhost(); out(" does not like recipient.\n");
      outsmtptext(); zero();
    }
    else {
      out("r"); zero();
      flagbother = 1;
    }
  }
  if (!flagbother) quit("DGiving up on ","");
 
  substdio_putsflush(&smtpto,"DATA\r\n");
  code = smtpcode();
  if (code >= 500) quit("D"," failed on DATA command");
  if (code >= 400) quit("Z"," failed on DATA command");
 
  if (enable_smtputf8 && header.len) substdio_put(&smtpto, header.s, header.len);
  blast();
  code = smtpcode();
  flagcritical = 0;
  if (code >= 500) quit("D"," failed after I sent the message");
  if (code >= 400) quit("Z"," failed after I sent the message");
  quit("K"," accepted message");
}

stralloc canonhost = {0};
stralloc canonbox = {0};

void addrmangle(stralloc *saout, char *s)
{
  int j;

  if (enable_smtputf8 && !flagutf8)
    flagutf8 = containsutf8(s,str_len(s));

  j = str_rchr(s,'@');
  if (!s[j]) {
    if (!stralloc_copys(saout,s)) temp_nomem();
    return;
  }
  if (!stralloc_copys(&canonbox,s)) temp_nomem();
  canonbox.len = j;
  /* box has to be quoted */
  if (!quote(saout,&canonbox)) temp_nomem();
  if (!stralloc_cats(saout,"@")) temp_nomem();
 
  if (!stralloc_copys(&canonhost,s + j + 1)) temp_nomem();

  if (!stralloc_cat(saout,&canonhost)) temp_nomem();
}

void getcontrols()
{
  if (control_init() == -1) temp_control();
  if (control_readint(&timeout,"control/timeoutremote") == -1) temp_control();
  if (control_readint(&timeoutconnect,"control/timeoutconnect") == -1)
    temp_control();
#ifdef SMTPUTF8
  if (control_readint(&enable_smtputf8,"control/smtputf8") == -1) temp_control();
#endif
  if (control_rldef(&helohost,"control/helohost",1,NULL) != 1)
    temp_control();
  switch(control_readfile(&routes,"control/smtproutes",0)) {
    case -1:
      temp_control();
    case 0:
      if (!constmap_init(&maproutes,"",0,1)) temp_nomem(); break;
    case 1:
      if (!constmap_init(&maproutes,routes.s,routes.len,1)) temp_nomem(); break;
  }
}

int main(int argc, char **argv)
{
  static ipalloc ip = {0};
  int i;
  unsigned long random;
  char **recips;
  unsigned long prefme;
  char *relayhost;
 
  sig_pipeignore();
  if (argc < 4) perm_usage();
  if (chdir(auto_qmail) == -1) temp_chdir();
  getcontrols();
 
  if (!stralloc_copys(&host,argv[1])) temp_nomem();
 
  relayhost = 0;
  for (i = 0;i <= host.len;++i)
    if ((i == 0) || (i == host.len) || (host.s[i] == '.'))
      if ((relayhost = constmap(&maproutes,host.s + i,host.len - i)))
        break;
  if (relayhost && !*relayhost) relayhost = 0;
 
  if (relayhost) {
    i = str_chr(relayhost,':');
    if (relayhost[i]) {
      scan_ulong(relayhost + i + 1,&port);
      relayhost[i] = 0;
    }
    if (!stralloc_copys(&host,relayhost)) temp_nomem();
  }
#ifdef SMTPUTF8
  else
  if (enable_smtputf8) {
      char *asciihost = NULL;
      if (!stralloc_0(&host)) temp_nomem();
      switch (idn2_lookup_u8(host.s,(uint8_t**)&asciihost,IDN2_NFC_INPUT)) {
        case IDN2_OK:     break;
        case IDN2_MALLOC: temp_nomem();
        default:          perm_dns();
      }
      if (!stralloc_copys(&idnhost,asciihost)) temp_nomem();
  }
#endif

  /*- addrmangle also sets flagutf8 */
  addrmangle(&sender,argv[2]);
 
  if (!saa_readyplus(&reciplist,0)) temp_nomem();
  if (ipme_init() != 1) temp_oserr();
 
  recips = argv + 3;
  while (*recips) {
    if (!saa_readyplus(&reciplist,1)) temp_nomem();
    reciplist.sa[reciplist.len] = sauninit;
    addrmangle(reciplist.sa + reciplist.len,*recips);
    ++reciplist.len;
    ++recips;
  }

 
  random = now() + (getpid() << 16);
  i = relayhost ? dns_ip(&ip,&host) : dns_mxip(&ip,enable_smtputf8 ? &idnhost : &host,random);
  switch (i) {
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
 
  for (i = 0;i < ip.len;++i)
    if (ip.ix[i].pref < prefme)
      break;
 
  if (i >= ip.len)
    perm_ambigmx();
 
  for (i = 0;i < ip.len;++i) if (ip.ix[i].pref < prefme) {
    if (tcpto(&ip.ix[i].ip)) continue;
 
    smtpfd = socket(AF_INET,SOCK_STREAM,0);
    if (smtpfd == -1) temp_oserr();
 
    if (timeoutconn(smtpfd,&ip.ix[i].ip,(unsigned int) port,timeoutconnect) == 0) {
      tcpto_err(&ip.ix[i].ip,0);
      partner = ip.ix[i].ip;
      smtp(); /* does not return */
    }
    tcpto_err(&ip.ix[i].ip,errno == error_timeout);
    close(smtpfd);
  }
  
  temp_noconn();
}
