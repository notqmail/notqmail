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
#ifndef TLS
#include "timeoutread.h"
#include "timeoutwrite.h"
#endif

#ifdef TLS
#include <sys/stat.h>
#include <openssl/ssl.h>
SSL *ssl = NULL;
#endif

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

#ifdef TLS
int flagtimedout = 0;
void sigalrm()
{
 flagtimedout = 1;
}
int ssl_timeoutread(timeout,fd,buf,n) int timeout; int fd; char *buf; int n;
{
 int r; int saveerrno;
 if (flagtimedout) { errno = error_timeout; return -1; }
 alarm(timeout);
 if (ssl) r = SSL_read(ssl,buf,n); else r = read(fd,buf,n);
 saveerrno = errno;
 alarm(0);
 if (flagtimedout) { errno = error_timeout; return -1; }
 errno = saveerrno;
 return r;
}
int ssl_timeoutwrite(timeout,fd,buf,n) int timeout; int fd; char *buf; int n;
{
 int r; int saveerrno;
 if (flagtimedout) { errno = error_timeout; return -1; }
 alarm(timeout);
 if (ssl) r = SSL_write(ssl,buf,n); else r = write(fd,buf,n);
 saveerrno = errno;
 alarm(0);
 if (flagtimedout) { errno = error_timeout; return -1; }
 errno = saveerrno;
 return r;
}
#endif 

int saferead(fd,buf,len) int fd; char *buf; int len;
{
  int r;
#ifdef TLS
  r = ssl_timeoutread(timeout,smtpfd,buf,len);
#else
  r = timeoutread(timeout,smtpfd,buf,len);
#endif
  if (r <= 0) dropped();
  return r;
}
int safewrite(fd,buf,len) int fd; char *buf; int len;
{
  int r;
#ifdef TLS
  r = ssl_timeoutwrite(timeout,smtpfd,buf,len);
#else
  r = timeoutwrite(timeout,smtpfd,buf,len);
#endif
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
/* TAG */
#if defined(TLS) && defined(DEBUG)
#define ONELINE_NAME(X) X509_NAME_oneline(X,NULL,0)

 if(ssl){
 X509 *peer;

  out("STARTTLS proto="); out(SSL_get_version(ssl));
  out("; cipher="); out(SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)));

  /* we want certificate details */
  peer=SSL_get_peer_certificate(ssl);
  if (peer != NULL) {
   char *str;

   str=ONELINE_NAME(X509_get_subject_name(peer));
   out("; subject="); out(str);
   Free(str);
   str=ONELINE_NAME(X509_get_issuer_name(peer));
   out("; issuer="); out(str);
   Free(str);
   X509_free(peer);
  }
  out(";\n");
 }
#endif

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

#ifdef TLS
void smtp(fqdn)
char *fqdn;
#else
void smtp()
#endif
{
  unsigned long code;
  int flagbother;
  int i;
#ifdef TLS
  int needtlsauth = 0;
  SSL_CTX *ctx;
  int saveerrno, r;
#ifdef DEBUG
  char buf[1024];
#endif

  stralloc servercert = {0};
  struct stat st;

  if(!stralloc_copys(&servercert, "control/tlshosts/")) temp_nomem();
  if(!stralloc_catb(&servercert, fqdn, str_len(fqdn))) temp_nomem();
  if(!stralloc_catb(&servercert, ".pem", 4)) temp_nomem();
  if(!stralloc_0(&servercert)) temp_nomem();
  if (stat(servercert.s,&st) == 0)  needtlsauth = 1;
#endif

  if (smtpcode() != 220) quit("ZConnected to "," but greeting failed");
 
#ifdef TLS
  substdio_puts(&smtpto,"EHLO ");
#else
  substdio_puts(&smtpto,"HELO ");
#endif
  substdio_put(&smtpto,helohost.s,helohost.len);
  substdio_puts(&smtpto,"\r\n");
  substdio_flush(&smtpto);
#ifdef TLS
  if (smtpcode() != 250){
   substdio_puts(&smtpto,"HELO ");
   substdio_put(&smtpto,helohost.s,helohost.len);
   substdio_puts(&smtpto,"\r\n");
   substdio_flush(&smtpto);
   if (smtpcode() != 250) quit("ZConnected to "," but my name was rejected");
  }
#else
  if (smtpcode() != 250) quit("ZConnected to "," but my name was rejected");
#endif

#ifdef TLS
  i = 0; 
  while((i += str_chr(smtptext.s+i,'\n') + 1) && (i+12 < smtptext.len) &&
        str_diffn(smtptext.s+i+4,"STARTTLS\n",9));
  if (i+12 < smtptext.len)
   {
    substdio_puts(&smtpto,"STARTTLS\r\n");
    substdio_flush(&smtpto);
    if (smtpcode() == 220)
     {
#ifdef DEBUG
      SSL_load_error_strings();
#endif
      SSLeay_add_ssl_algorithms();
      if(!(ctx=SSL_CTX_new(SSLv23_client_method())))
#ifdef DEBUG
       {out("ZTLS not available: error initializing ctx");
        out(": ");
        out(ERR_error_string(ERR_get_error(), buf));
        out("\n");
#else
       {out("ZTLS not available: error initializing ctx\n");
#endif
         zerodie();}

      SSL_CTX_use_RSAPrivateKey_file(ctx, "control/cert.pem", SSL_FILETYPE_PEM);
      SSL_CTX_use_certificate_file(ctx, "control/cert.pem", SSL_FILETYPE_PEM);
      /*SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1);*/

      if (needtlsauth){
        if (!SSL_CTX_load_verify_locations(ctx, servercert.s, NULL))
          {out("ZTLS unable to load "); out(servercert.s); out("\n");
           zerodie();}
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
      }
     
      if(!(ssl=SSL_new(ctx)))
#ifdef DEBUG
        {out("ZTLS not available: error initializing ssl");
         out(": ");
         out(ERR_error_string(ERR_get_error(), buf));
         out("\n");
#else
        {out("ZTLS not available: error initializing ssl\n");
#endif
         zerodie();}
      SSL_set_fd(ssl,smtpfd);

      alarm(timeout);
      r = SSL_connect(ssl); saveerrno = errno;
      alarm(0); 
      if (flagtimedout) 
       {out("ZTLS not available: connect timed out\n");
        zerodie();}
      errno = saveerrno;
      if (r<=0)
       {if (needtlsauth && (r=SSL_get_verify_result(ssl)) != X509_V_OK)
         {out("ZTLS unable to verify server with ");
          out(servercert.s); out(": ");
          out(X509_verify_cert_error_string(r)); out("\n");}
        else
#ifdef DEBUG
         {out("ZTLS not available: connect failed");
          out(": ");
          out(ERR_error_string(ERR_get_error(), buf));
          out("\n");}
#else
         out("ZTLS not available: connect failed\n");
#endif
        zerodie();}
      if (needtlsauth)
       /* should also check alternate names */
       {char commonName[256];
        X509_NAME_get_text_by_NID(X509_get_subject_name(
                                   SSL_get_peer_certificate(ssl)),
                                   NID_commonName, commonName, 256);
        if (strcasecmp(fqdn,commonName)){
         out("ZTLS connection to "); out(fqdn);
         out(" wanted, certificate for "); out(commonName);
         out(" received\n");
         zerodie();}
        }

      substdio_puts(&smtpto,"EHLO ");
      substdio_put(&smtpto,helohost.s,helohost.len);
      substdio_puts(&smtpto,"\r\n");
      substdio_flush(&smtpto);

      if (smtpcode() != 250)
       {
        quit("ZTLS connected to "," but my name was rejected");
       }
     } 
   }
  if ((!ssl) && needtlsauth)
   {out("ZNo TLS achieved while "); out(servercert.s); out(" exists.\n");
    quit();}
#endif

  substdio_puts(&smtpto,"MAIL FROM:<");
  substdio_put(&smtpto,sender.s,sender.len);
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

#ifdef TLS
  sig_alarmcatch(sigalrm);
#endif
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
 
  if (relayhost) {
    i = str_chr(relayhost,':');
    if (relayhost[i]) {
      scan_ulong(relayhost + i + 1,&port);
      relayhost[i] = 0;
    }
    if (!stralloc_copys(&host,relayhost)) temp_nomem();
  }


  addrmangle(&sender,argv[2],&flagalias,0);
 
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
#ifdef TLS
      smtp(ip.ix[i].fqdn); /* does not return */
#else
      smtp(); /* does not return */
#endif
    }
    tcpto_err(&ip.ix[i].ip,errno == error_timeout);
    close(smtpfd);
  }
  
  temp_noconn();
}
