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
#include "readwrite.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "base64.h"

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

stralloc authsenders = {0};
struct constmap mapauthsenders;
stralloc user = {0};
stralloc pass = {0};
stralloc auth = {0};
stralloc plain = {0};
stralloc chal  = {0};
stralloc slop  = {0};
char *authsender;

saa reciplist = {0};

struct ip_address partner;

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

void err_authprot() {
  out("Kno supported AUTH method found, continuing without authentication.\n");
  zero();
  substdio_flush(subfdoutsmall);
}

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

int saferead(fd,buf,len) int fd; char *buf; int len;
{
  int r;
  r = timeoutread(timeout,smtpfd,buf,len);
  if (r <= 0) dropped();
  return r;
}
int safewrite(fd,buf,len) int fd; char *buf; int len;
{
  int r;
  r = timeoutwrite(timeout,smtpfd,buf,len);
  if (r <= 0) dropped();
  return r;
}

char inbuf[1024];
substdio ssin = SUBSTDIO_FDBUF(read,0,inbuf,sizeof inbuf);
char smtptobuf[1024];
substdio smtpto = SUBSTDIO_FDBUF(safewrite,-1,smtptobuf,sizeof smtptobuf);
char smtpfrombuf[128];
substdio smtpfrom = SUBSTDIO_FDBUF(saferead,-1,smtpfrombuf,sizeof smtpfrombuf);

stralloc smtptext = {0};

void get(ch)
char *ch;
{
  substdio_get(&smtpfrom,ch,1);
  if (*ch != '\r')
    if (smtptext.len < HUGESMTPTEXT)
     if (!stralloc_append(&smtptext,ch)) temp_nomem();
}

unsigned long smtpcode()
{
  unsigned char ch;
  unsigned long code;

  if (!stralloc_copys(&smtptext,"")) temp_nomem();

  get(&ch); code = ch - '0';
  get(&ch); code = code * 10 + (ch - '0');
  get(&ch); code = code * 10 + (ch - '0');
  for (;;) {
    get(&ch);
    if (ch != '-') break;
    while (ch != '\n') get(&ch);
    get(&ch);
    get(&ch);
    get(&ch);
  }
  while (ch != '\n') get(&ch);

  return code;
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

stralloc recip = {0};

void mailfrom()
{
  substdio_puts(&smtpto,"MAIL FROM:<");
  substdio_put(&smtpto,sender.s,sender.len);
  substdio_puts(&smtpto,">\r\n");
  substdio_flush(&smtpto);
}

stralloc xuser = {0};

int xtext(sa,s,len)
stralloc *sa;
char *s;
int len;
{
  int i;

  if(!stralloc_copys(sa,"")) temp_nomem();

  for (i = 0; i < len; i++) {
    if (s[i] == '=') {
      if (!stralloc_cats(sa,"+3D")) temp_nomem();
    } else if (s[i] == '+') {
        if (!stralloc_cats(sa,"+2B")) temp_nomem();
    } else if ((int) s[i] < 33 || (int) s[i] > 126) {
        if (!stralloc_cats(sa,"+3F")) temp_nomem(); /* ok. not correct */
    } else if (!stralloc_catb(sa,s+i,1)) {
        temp_nomem();
    }
  }

  return sa->len;
}

void mailfrom_xtext()
{
  if (!xtext(&xuser,user.s,user.len)) temp_nomem();
  substdio_puts(&smtpto,"MAIL FROM:<");
  substdio_put(&smtpto,sender.s,sender.len);
  substdio_puts(&smtpto,"> AUTH=");
  substdio_put(&smtpto,xuser.s,xuser.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);
}

int mailfrom_plain()
{
  substdio_puts(&smtpto,"AUTH PLAIN\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() != 334) { quit("ZConnected to "," but authentication was rejected (AUTH PLAIN)."); return -1; }

  if (!stralloc_cat(&plain,&user)) temp_nomem(); /* <authorization-id> */
  if (!stralloc_0(&plain)) temp_nomem();
  if (!stralloc_cat(&plain,&user)) temp_nomem(); /* <authentication-id> */
  if (!stralloc_0(&plain)) temp_nomem();
  if (!stralloc_cat(&plain,&pass)) temp_nomem(); /* password */
  if (b64encode(&plain,&auth)) quit("ZConnected to "," but unable to base64encode (plain).");
  substdio_put(&smtpto,auth.s,auth.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() == 235) { mailfrom_xtext(); return 0; }
  else if (smtpcode() == 432) { quit("ZConnected to "," but password expired."); return 1; }
  else { quit("ZConnected to "," but authentication was rejected (plain)."); return 1; }

  return 0;
}

int mailfrom_login()
{
  substdio_puts(&smtpto,"AUTH LOGIN\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() != 334) { quit("ZConnected to "," but authentication was rejected (AUTH LOGIN)."); return -1; }

  if (!stralloc_copys(&auth,"")) temp_nomem();
  if (b64encode(&user,&auth)) quit("ZConnected to "," but unable to base64encode user.");
  substdio_put(&smtpto,auth.s,auth.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() != 334) quit("ZConnected to "," but authentication was rejected (username).");

  if (!stralloc_copys(&auth,"")) temp_nomem();
  if (b64encode(&pass,&auth)) quit("ZConnected to "," but unable to base64encode pass.");
  substdio_put(&smtpto,auth.s,auth.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() == 235) { mailfrom_xtext(); return 0; }
  else if (smtpcode() == 432) { quit("ZConnected to "," but password expired."); return 1; }
  else { quit("ZConnected to "," but authentication was rejected (password)."); return 1; }
}

int mailfrom_cram()
{
  int j;
  unsigned char h;
  unsigned char digest[16];
  unsigned char digascii[33];
  static char hextab[]="0123456789abcdef";

  substdio_puts(&smtpto,"AUTH CRAM-MD5\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() != 334) { quit("ZConnected to "," but authentication was rejected (AUTH CRAM-MD5)."); return -1; }

  if (str_chr(smtptext.s+4,' ')) { 			/* Challenge */
    if(!stralloc_copys(&slop,"")) temp_nomem();
    if (!stralloc_copyb(&slop,smtptext.s+4,smtptext.len-5)) temp_nomem();
    if (b64decode(slop.s,slop.len,&chal)) quit("ZConnected to "," but unable to base64decode challenge.");
  }

  hmac_md5(chal.s,chal.len,pass.s,pass.len,digest);

  for (j = 0;j < 16;j++)				/* HEX => ASCII */
  {
    digascii[2*j] = hextab[digest[j] >> 4];
    digascii[2*j+1] = hextab[digest[j] & 0xf];
  }
  digascii[32]=0;

  slop.len = 0;
  if (!stralloc_copys(&slop,"")) temp_nomem();
  if (!stralloc_cat(&slop,&user)) temp_nomem();		 /* user-id */
  if (!stralloc_cats(&slop," ")) temp_nomem();
  if (!stralloc_catb(&slop,digascii,32)) temp_nomem();   /* digest */

  if (!stralloc_copys(&auth,"")) temp_nomem();
  if (b64encode(&slop,&auth)) quit("ZConnected to "," but unable to base64encode username+digest.");
  substdio_put(&smtpto,auth.s,auth.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);
  if (smtpcode() == 235) { mailfrom_xtext(); return 0; }
  else if (smtpcode() == 432) { quit("ZConnected to "," but password expired."); return 1; }
  else { quit("ZConnected to "," but authentication was rejected (username+digest)."); return 1; }
}

void smtp_auth()
{
  int i, j;

  for (i = 0; i + 8 < smtptext.len; i += str_chr(smtptext.s+i,'\n')+1)
    if (!str_diffn(smtptext.s+i+4,"AUTH",4)) {
      if (j = str_chr(smtptext.s+i+8,'C') > 0)
        if (case_starts(smtptext.s+i+8+j,"CRAM"))
          if (mailfrom_cram() >= 0) return;

      if (j = str_chr(smtptext.s+i+8,'P') > 0)
        if (case_starts(smtptext.s+i+8+j,"PLAIN"))
          if (mailfrom_plain() >= 0) return;

      if (j = str_chr(smtptext.s+i+8,'L') > 0)
        if (case_starts(smtptext.s+i+8+j,"LOGIN"))
          if (mailfrom_login() >= 0) return;

      err_authprot();
      mailfrom();
    }
}

void smtp()
{
  unsigned long code;
  int flagbother;
  int i;
 
  code = smtpcode();
  if (code >= 500) quit("DConnected to "," but greeting failed");
  if (code != 220) quit("ZConnected to "," but greeting failed");
 
  substdio_puts(&smtpto,"EHLO ");
  substdio_put(&smtpto,helohost.s,helohost.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);

  if (smtpcode() != 250) {
    substdio_puts(&smtpto,"HELO ");
    substdio_put(&smtpto,helohost.s,helohost.len);
    substdio_puts(&smtpto,"\r\n");
    substdio_flush(&smtpto);
    code = smtpcode();
    if (code >= 500) quit("DConnected to "," but my name was rejected");
    if (code != 250) quit("ZConnected to "," but my name was rejected");
  }

  if (user.len && pass.len)
    smtp_auth();
  else
    mailfrom();

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
 
  blast();
  code = smtpcode();
  flagcritical = 0;
  if (code >= 500) quit("D"," failed after I sent the message");
  if (code >= 400) quit("Z"," failed after I sent the message");
  quit("K"," accepted message");
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
  if (!s[j]) {
    if (!stralloc_copys(saout,s)) temp_nomem();
    return;
  }
  if (!stralloc_copys(&canonbox,s)) temp_nomem();
  canonbox.len = j;
  if (!quote(saout,&canonbox)) temp_nomem();
  if (!stralloc_cats(saout,"@")) temp_nomem();
 
  if (!stralloc_copys(&canonhost,s + j + 1)) temp_nomem();
  if (flagcname)
    switch(dns_cname(&canonhost)) {
      case 0: *flagalias = 0; break;
      case DNS_MEM: temp_nomem();
      case DNS_SOFT: temp_dnscanon();
      case DNS_HARD: ; /* alias loop, not our problem */
    }

  if (!stralloc_cat(saout,&canonhost)) temp_nomem();
}

void getcontrols()
{
  if (control_init() == -1) temp_control();
  if (control_readint(&timeout,"control/timeoutremote") == -1) temp_control();
  if (control_readint(&timeoutconnect,"control/timeoutconnect") == -1)
    temp_control();
  if (control_rldef(&helohost,"control/helohost",1,(char *) 0) != 1)
    temp_control();
  switch(control_readfile(&routes,"control/smtproutes",0)) {
    case -1:
      temp_control();
    case 0:
      if (!constmap_init(&maproutes,"",0,1)) temp_nomem(); break;
    case 1:
      if (!constmap_init(&maproutes,routes.s,routes.len,1)) temp_nomem(); break;
  }

  switch(control_readfile(&authsenders,"control/authsenders",0)) {
    case -1:
       temp_control();
    case 0:
      if (!constmap_init(&mapauthsenders,"",0,1)) temp_nomem(); break;
    case 1:
      if (!constmap_init(&mapauthsenders,authsenders.s,authsenders.len,1)) temp_nomem(); break;
  }
}

int main(argc,argv)
int argc;
char **argv;
{
  static ipalloc ip = {0};
  int i, j;
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

  authsender = 0;
  relayhost = 0;

  addrmangle(&sender,argv[2],&flagalias,0);

  for (i = 0;i <= sender.len;++i)
    if ((i == 0) || (i == sender.len) || (sender.s[i] == '.') || (sender.s[i] == '@'))
      if (authsender = constmap(&mapauthsenders,sender.s + i,sender.len - i))
        break;

  if (authsender && !*authsender) authsender = 0;

  if (authsender) {
    i = str_chr(authsender,'|');
    if (authsender[i]) {
      j = str_chr(authsender + i + 1,'|');
      if (authsender[j]) {
        authsender[i] = 0;
        authsender[i + j + 1] = 0;
        if (!stralloc_copys(&user,"")) temp_nomem();
        if (!stralloc_copys(&user,authsender + i + 1)) temp_nomem();
        if (!stralloc_copys(&pass,"")) temp_nomem();
        if (!stralloc_copys(&pass,authsender + i + j + 2)) temp_nomem();
      }
    }
    i = str_chr(authsender,':');
    if (authsender[i]) {
      scan_ulong(authsender + i + 1,&port);
      authsender[i] = 0;
    }

    if (!stralloc_copys(&relayhost,authsender)) temp_nomem();
    if (!stralloc_copys(&host,authsender)) temp_nomem();

  }
  else {					/* default smtproutes -- authenticated */
    for (i = 0;i <= host.len;++i)
      if ((i == 0) || (i == host.len) || (host.s[i] == '.'))
        if (relayhost = constmap(&maproutes,host.s + i,host.len - i))
          break;

    if (relayhost && !*relayhost) relayhost = 0;

    if (relayhost) {
      i = str_chr(relayhost,'|');
      if (relayhost[i]) {
        j = str_chr(relayhost + i + 1,'|');
        if (relayhost[j]) {
          relayhost[i] = 0;
          relayhost[i + j + 1] = 0;
          if (!stralloc_copys(&user,"")) temp_nomem();
          if (!stralloc_copys(&user,relayhost + i + 1)) temp_nomem();
          if (!stralloc_copys(&pass,"")) temp_nomem();
          if (!stralloc_copys(&pass,relayhost + i + j + 2)) temp_nomem();
        }
      }
      i = str_chr(relayhost,':');
      if (relayhost[i]) {
        scan_ulong(relayhost + i + 1,&port);
        relayhost[i] = 0;
      }
      if (!stralloc_copys(&host,relayhost)) temp_nomem();
    }
  }

  if (!saa_readyplus(&reciplist,0)) temp_nomem();
  if (ipme_init() != 1) temp_oserr();
 
  flagallaliases = 1;
  recips = argv + 3;
  while (*recips) {
    if (!saa_readyplus(&reciplist,1)) temp_nomem();
    reciplist.sa[reciplist.len] = sauninit;
    addrmangle(reciplist.sa + reciplist.len,*recips,&flagalias,!relayhost);
    if (!flagalias) flagallaliases = 0;
    ++reciplist.len;
    ++recips;
  }

 
  random = now() + (getpid() << 16);
  switch (relayhost ? dns_ip(&ip,&host) : dns_mxip(&ip,&host,random)) {
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
