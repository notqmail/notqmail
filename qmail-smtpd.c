#include "sig.h"
#include "readwrite.h"
#include "stralloc.h"
#include "substdio.h"
#include "alloc.h"
#include "auto_qmail.h"
#include "control.h"
#include "received.h"
#include "constmap.h"
#include "error.h"
#include "ipme.h"
#include "ip.h"
#include "qmail.h"
#include "str.h"
#include "fmt.h"
#include "scan.h"
#include "byte.h"
#include "case.h"
#include "env.h"
#include "now.h"
#include "exit.h"
#include "rcpthosts.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "commands.h"

#define MAXHOPS 100
unsigned int databytes = 0;
int timeout = 1200;

#ifdef TLS

#include "ssl_timeoutio.h"
#include <openssl/ssl.h>

SSL *ssl = NULL;
/* SSL_get_Xfd() are broken */
int ssl_rfd = -1, ssl_wfd = -1;

int smtps = 0;
void init_tls();

stralloc clientcert = {0};
stralloc tlsserverciphers = {0};

static int verify_cb(int ok, X509_STORE_CTX * ctx)
{
  return 1;
}

#endif

int safewrite(fd,buf,len) int fd; char *buf; int len;
{
  int r;
#ifdef TLS
  if (ssl && fd == ssl_wfd)
    r = ssl_timeoutwrite(timeout,ssl_rfd,ssl_wfd,ssl,buf,len);
  else
#endif
  r = timeoutwrite(timeout,fd,buf,len);
  if (r <= 0) _exit(1);
  return r;
}

char ssoutbuf[512];
substdio ssout = SUBSTDIO_FDBUF(safewrite,1,ssoutbuf,sizeof ssoutbuf);

void flush() { substdio_flush(&ssout); }
void out(s) char *s; { substdio_puts(&ssout,s); }

void die_read() { _exit(1); }
void die_alarm() { out("451 timeout (#4.4.2)\r\n"); flush(); _exit(1); }
void die_nomem() { out("421 out of memory (#4.3.0)\r\n"); flush(); _exit(1); }
void die_control() { out("421 unable to read controls (#4.3.0)\r\n"); flush(); _exit(1); }
void die_ipme() { out("421 unable to figure out my IP addresses (#4.3.0)\r\n"); flush(); _exit(1); }
void straynewline() { out("451 See http://pobox.com/~djb/docs/smtplf.html.\r\n"); flush(); _exit(1); }

void err_bmf() { out("553 sorry, your envelope sender is in my badmailfrom list (#5.7.1)\r\n"); }
void err_nogateway() { out("553 sorry, that domain isn't in my list of allowed rcpthosts (#5.7.1)\r\n"); }
#ifdef TLS
void err_nogwcert(const char *s)
{
  out("553 no valid cert for gatewaying");
  if (s) { out(": "); out(s); }
  out(" (#5.7.1)\r\n");
}
#endif
void err_unimpl() { out("502 unimplemented (#5.5.1)\r\n"); }
void err_syntax() { out("555 syntax error (#5.5.4)\r\n"); }
void err_wantmail() { out("503 MAIL first (#5.5.1)\r\n"); }
void err_wantrcpt() { out("503 RCPT first (#5.5.1)\r\n"); }
void err_noop() { out("250 ok\r\n"); }
void err_vrfy() { out("252 send some mail, i'll try my best\r\n"); }
void err_qqt() { out("451 qqt failure (#4.3.0)\r\n"); }


stralloc greeting = {0};

void smtp_greet(code) char *code;
{
  substdio_puts(&ssout,code);
  substdio_put(&ssout,greeting.s,greeting.len);
}
void smtp_help()
{
  out("214 qmail home page: http://pobox.com/~djb/qmail.html\r\n");
}
void smtp_quit()
{
  smtp_greet("221 "); out("\r\n"); flush(); _exit(0);
}

char *remoteip;
char *remotehost;
char *remoteinfo;
char *local;
char *relayclient;

stralloc helohost = {0};
char *fakehelo; /* pointer into helohost, or 0 */

void dohelo(arg) char *arg; {
  if (!stralloc_copys(&helohost,arg)) die_nomem(); 
  if (!stralloc_0(&helohost)) die_nomem(); 
  fakehelo = case_diffs(remotehost,helohost.s) ? helohost.s : 0;
}

int liphostok = 0;
stralloc liphost = {0};
int bmfok = 0;
stralloc bmf = {0};
struct constmap mapbmf;

void setup()
{
  char *x;
  unsigned long u;
 
  if (control_init() == -1) die_control();
  if (control_rldef(&greeting,"control/smtpgreeting",1,(char *) 0) != 1)
    die_control();
  liphostok = control_rldef(&liphost,"control/localiphost",1,(char *) 0);
  if (liphostok == -1) die_control();
  if (control_readint(&timeout,"control/timeoutsmtpd") == -1) die_control();
  if (timeout <= 0) timeout = 1;

  if (rcpthosts_init() == -1) die_control();

  bmfok = control_readfile(&bmf,"control/badmailfrom",0);
  if (bmfok == -1) die_control();
  if (bmfok)
    if (!constmap_init(&mapbmf,bmf.s,bmf.len,0)) die_nomem();
 
  if (control_readint(&databytes,"control/databytes") == -1) die_control();
  x = env_get("DATABYTES");
  if (x) { scan_ulong(x,&u); databytes = u; }
  if (!(databytes + 1)) --databytes;
 
  remoteip = env_get("TCPREMOTEIP");
  if (!remoteip) remoteip = "unknown";
  local = env_get("TCPLOCALHOST");
  if (!local) local = env_get("TCPLOCALIP");
  if (!local) local = "unknown";
  remotehost = env_get("TCPREMOTEHOST");
  if (!remotehost) remotehost = "unknown";
  remoteinfo = env_get("TCPREMOTEINFO");
  relayclient = env_get("RELAYCLIENT");

#ifdef TLS
  x = env_get("TLSCIPHERS");
  if (x) { if (*x) if (!stralloc_copys(&tlsserverciphers,x)) die_nomem(); } 
  else if (control_readline(&tlsserverciphers,"control/tlsserverciphers") == -1)
    die_control();
  if (!tlsserverciphers.len)
    if (!stralloc_copys(&tlsserverciphers,"DEFAULT")) die_nomem();
  if (!stralloc_0(&tlsserverciphers)) die_nomem();

  x = env_get("SMTPS");
  if (x && *x) {
    smtps = 1;
    init_tls();
  }
  else
#endif
  dohelo(remotehost);
}


stralloc addr = {0}; /* will be 0-terminated, if addrparse returns 1 */

int addrparse(arg)
char *arg;
{
  int i;
  char ch;
  char terminator;
  struct ip_address ip;
  int flagesc;
  int flagquoted;
 
  terminator = '>';
  i = str_chr(arg,'<');
  if (arg[i])
    arg += i + 1;
  else { /* partner should go read rfc 821 */
    terminator = ' ';
    arg += str_chr(arg,':');
    if (*arg == ':') ++arg;
    while (*arg == ' ') ++arg;
  }

  /* strip source route */
  if (*arg == '@') while (*arg) if (*arg++ == ':') break;

  if (!stralloc_copys(&addr,"")) die_nomem();
  flagesc = 0;
  flagquoted = 0;
  for (i = 0;ch = arg[i];++i) { /* copy arg to addr, stripping quotes */
    if (flagesc) {
      if (!stralloc_append(&addr,&ch)) die_nomem();
      flagesc = 0;
    }
    else {
      if (!flagquoted && (ch == terminator)) break;
      switch(ch) {
        case '\\': flagesc = 1; break;
        case '"': flagquoted = !flagquoted; break;
        default: if (!stralloc_append(&addr,&ch)) die_nomem();
      }
    }
  }
  /* could check for termination failure here, but why bother? */
  if (!stralloc_append(&addr,"")) die_nomem();

  if (liphostok) {
    i = byte_rchr(addr.s,addr.len,'@');
    if (i < addr.len) /* if not, partner should go read rfc 821 */
      if (addr.s[i + 1] == '[')
        if (!addr.s[i + 1 + ip_scanbracket(addr.s + i + 1,&ip)])
          if (ipme_is(&ip)) {
            addr.len = i + 1;
            if (!stralloc_cat(&addr,&liphost)) die_nomem();
            if (!stralloc_0(&addr)) die_nomem();
          }
  }

  if (addr.len > 900) return 0;
  return 1;
}

int bmfcheck()
{
  int j;
  if (!bmfok) return 0;
  if (constmap(&mapbmf,addr.s,addr.len - 1)) return 1;
  j = byte_rchr(addr.s,addr.len,'@');
  if (j < addr.len)
    if (constmap(&mapbmf,addr.s + j,addr.len - j - 1)) return 1;
  return 0;
}

int addrallowed()
{
  int r;
  r = rcpthosts(addr.s,str_len(addr.s));
  if (r == -1) die_control();
  return r;
}


int seenmail = 0;
int flagbarf; /* defined if seenmail */
stralloc mailfrom = {0};
stralloc rcptto = {0};

void smtp_helo(arg) char *arg;
{
  smtp_greet("250 "); out("\r\n");
  seenmail = 0; dohelo(arg);
}
/* ESMTP extensions are published here */
void smtp_ehlo(arg) char *arg;
{
  smtp_greet("250-");
#ifdef TLS
  if (!ssl) out("\r\n250-STARTTLS");
#endif
  out("\r\n250-PIPELINING\r\n250 8BITMIME\r\n");
  seenmail = 0; dohelo(arg);
}
void smtp_rset()
{
  seenmail = 0;
  out("250 flushed\r\n");
}
void smtp_mail(arg) char *arg;
{
  if (!addrparse(arg)) { err_syntax(); return; }
  flagbarf = bmfcheck();
  seenmail = 1;
  if (!stralloc_copys(&rcptto,"")) die_nomem();
  if (!stralloc_copys(&mailfrom,addr.s)) die_nomem();
  if (!stralloc_0(&mailfrom)) die_nomem();
  out("250 ok\r\n");
}
void smtp_rcpt(arg) char *arg; {
  if (!seenmail) { err_wantmail(); return; }
  if (!addrparse(arg)) { err_syntax(); return; }
  if (flagbarf) { err_bmf(); return; }
  if (relayclient) {
    --addr.len;
    if (!stralloc_cats(&addr,relayclient)) die_nomem();
    if (!stralloc_0(&addr)) die_nomem();
  }
  else
    if (!addrallowed()) {
#ifdef TLS
    static int checkcert = 1;
    stralloc tlsclients;
    struct constmap maptlsclients;

    if (ssl && checkcert) {
      STACK_OF(X509_NAME) *sk;
      checkcert = 0;
      if (control_readfile(&tlsclients,"control/tlsclients",0) == 1)
        switch (constmap_init(&maptlsclients,tlsclients.s,tlsclients.len,0))
        {
        default:
          if (sk = SSL_load_client_CA_file("control/clientca.pem")) {
            checkcert = 1;
            SSL_set_client_CA_list(ssl, sk);
            break;
          }
          constmap_free(&maptlsclients);
        case 0:
          alloc_free(tlsclients.s);
        }
    }
    if (ssl && checkcert) {
      const char *errstr = NULL;
      checkcert = 0;
      do { /* renegotiate with the client */
        int r;
        X509 *peercert;
        char emailAddress[256];

        SSL_set_verify(ssl,SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE,verify_cb);
        if (SSL_renegotiate(ssl) <= 0) break;
        if (SSL_do_handshake(ssl) <= 0) break;
        /* SSL_set_accept_state() clears crypto state so don't use */
        ssl->state = SSL_ST_ACCEPT;
        if (SSL_do_handshake(ssl) <= 0) break;

        if ((r = SSL_get_verify_result(ssl)) != X509_V_OK) {
          errstr = X509_verify_cert_error_string(r);
          break;
        }
        if (!(peercert = SSL_get_peer_certificate(ssl))) break;

        X509_NAME_get_text_by_NID(X509_get_subject_name(peercert),
          NID_pkcs9_emailAddress, emailAddress, 256);
        if (!stralloc_copys(&clientcert, emailAddress)) die_nomem();
        X509_free(peercert);

        /* checked out; set relayclient so no need to check next "RCPT TO" */
        if (constmap(&maptlsclients,clientcert.s,clientcert.len))
          relayclient = "";
      } while (0);
      constmap_free(&maptlsclients);
      alloc_free(tlsclients.s);
      if (!relayclient) { err_nogwcert(errstr); return; }
    }
    else
#endif
    { err_nogateway(); return; }
  }
  if (!stralloc_cats(&rcptto,"T")) die_nomem();
  if (!stralloc_cats(&rcptto,addr.s)) die_nomem();
  if (!stralloc_0(&rcptto)) die_nomem();
  out("250 ok\r\n");
}


int saferead(fd,buf,len) int fd; char *buf; int len;
{
  int r;
  flush();
#ifdef TLS
  if (ssl && fd == ssl_rfd)
    r = ssl_timeoutread(timeout,ssl_rfd,ssl_wfd,ssl,buf,len);
  else
#endif
  r = timeoutread(timeout,fd,buf,len);
  if (r == -1) if (errno == error_timeout) die_alarm();
  if (r <= 0) die_read();
  return r;
}

char ssinbuf[1024];
substdio ssin = SUBSTDIO_FDBUF(saferead,0,ssinbuf,sizeof ssinbuf);

struct qmail qqt;
unsigned int bytestooverflow = 0;

void put(ch)
char *ch;
{
  if (bytestooverflow)
    if (!--bytestooverflow)
      qmail_fail(&qqt);
  qmail_put(&qqt,ch,1);
}

void blast(hops)
int *hops;
{
  char ch;
  int state;
  int flaginheader;
  int pos; /* number of bytes since most recent \n, if fih */
  int flagmaybex; /* 1 if this line might match RECEIVED, if fih */
  int flagmaybey; /* 1 if this line might match \r\n, if fih */
  int flagmaybez; /* 1 if this line might match DELIVERED, if fih */
 
  state = 1;
  *hops = 0;
  flaginheader = 1;
  pos = 0; flagmaybex = flagmaybey = flagmaybez = 1;
  for (;;) {
    substdio_get(&ssin,&ch,1);
    if (flaginheader) {
      if (pos < 9) {
        if (ch != "delivered"[pos]) if (ch != "DELIVERED"[pos]) flagmaybez = 0;
        if (flagmaybez) if (pos == 8) ++*hops;
        if (pos < 8)
          if (ch != "received"[pos]) if (ch != "RECEIVED"[pos]) flagmaybex = 0;
        if (flagmaybex) if (pos == 7) ++*hops;
        if (pos < 2) if (ch != "\r\n"[pos]) flagmaybey = 0;
        if (flagmaybey) if (pos == 1) flaginheader = 0;
      }
      ++pos;
      if (ch == '\n') { pos = 0; flagmaybex = flagmaybey = flagmaybez = 1; }
    }
    switch(state) {
      case 0:
        if (ch == '\n') straynewline();
        if (ch == '\r') { state = 4; continue; }
        break;
      case 1: /* \r\n */
        if (ch == '\n') straynewline();
        if (ch == '.') { state = 2; continue; }
        if (ch == '\r') { state = 4; continue; }
        state = 0;
        break;
      case 2: /* \r\n + . */
        if (ch == '\n') straynewline();
        if (ch == '\r') { state = 3; continue; }
        state = 0;
        break;
      case 3: /* \r\n + .\r */
        if (ch == '\n') return;
        put(".");
        put("\r");
        if (ch == '\r') { state = 4; continue; }
        state = 0;
        break;
      case 4: /* + \r */
        if (ch == '\n') { state = 1; break; }
        if (ch != '\r') { put("\r"); state = 0; }
    }
    put(&ch);
  }
}

char accept_buf[FMT_ULONG];
void acceptmessage(qp) unsigned long qp;
{
  datetime_sec when;
  when = now();
  out("250 ok ");
  accept_buf[fmt_ulong(accept_buf,(unsigned long) when)] = 0;
  out(accept_buf);
  out(" qp ");
  accept_buf[fmt_ulong(accept_buf,qp)] = 0;
  out(accept_buf);
  out("\r\n");
}

void smtp_data() {
  int hops;
  unsigned long qp;
  char *qqx;
 
  if (!seenmail) { err_wantmail(); return; }
  if (!rcptto.len) { err_wantrcpt(); return; }
  seenmail = 0;
  if (databytes) bytestooverflow = databytes + 1;
  if (qmail_open(&qqt) == -1) { err_qqt(); return; }
  qp = qmail_qp(&qqt);
  out("354 go ahead\r\n");
 
#ifdef TLS
  if (ssl) {
    static stralloc protocol = {0};
    if (!protocol.len) {
      if (!stralloc_copys(&protocol,SSL_get_cipher(ssl))) die_nomem();
      if (!stralloc_catb(&protocol," encrypted SMTP",15)) die_nomem();
      if (smtps) if (!stralloc_append(&protocol,"S")) die_nomem();
      if (clientcert.len) {
        if (!stralloc_catb(&protocol," cert ",6)) die_nomem();
        if (!stralloc_catb(&protocol,clientcert.s,clientcert.len)) die_nomem();
      }
      if (!stralloc_0(&protocol)) die_nomem();
    }
    received(&qqt,protocol.s,local,remoteip,remotehost,remoteinfo,fakehelo);
  } else
#endif
  received(&qqt,"SMTP",local,remoteip,remotehost,remoteinfo,fakehelo);
  blast(&hops);
  hops = (hops >= MAXHOPS);
  if (hops) qmail_fail(&qqt);
  qmail_from(&qqt,mailfrom.s);
  qmail_put(&qqt,rcptto.s,rcptto.len);
 
  qqx = qmail_close(&qqt);
  if (!*qqx) { acceptmessage(qp); return; }
  if (hops) { out("554 too many hops, this message is looping (#5.4.6)\r\n"); return; }
  if (databytes) if (!bytestooverflow) { out("552 sorry, that message size exceeds my databytes limit (#5.3.4)\r\n"); return; }
  if (*qqx == 'D') out("554 "); else out("451 ");
  out(qqx + 1);
  out("\r\n");
}

#ifdef TLS
void smtp_tls(arg) char *arg; 
{
  if (*arg) {
    out("501 Syntax error (no parameters allowed) (#5.5.4)\r\n");
    return;
  }
  init_tls();
}

static RSA *tmp_rsa_cb(ssl,export,keylen) SSL *ssl; int export; int keylen; 
{
  if (!export) keylen = 512;
  if (keylen == 512) {
    BIO *in = BIO_new_file("control/rsa512.pem", "r");
    if (in) {
      RSA *rsa = PEM_read_bio_RSAPrivateKey(in,NULL,NULL,NULL);
      BIO_free(in);
      if (rsa) return rsa;
    }
  }
  return RSA_generate_key(keylen,RSA_F4,NULL,NULL);
}

void init_tls()
{
  SSL_CTX *ctx;

  if (ssl) return;

  SSL_library_init();

  /* a new SSL context with the bare minimum of options */
  if (!(ctx = SSL_CTX_new(SSLv23_server_method()))) {
    out("454 TLS not available: unable to initialize ctx (#4.3.0)\r\n");
    flush();
    if (smtps) die_read();
    return;
  }
  if (!SSL_CTX_use_certificate_chain_file(ctx,"control/servercert.pem")) {
    out("454 TLS not available: missing certificate (#4.3.0)\r\n");
    flush();
    if (smtps) die_read();
    SSL_CTX_free(ctx);
    return;
  }
  SSL_CTX_load_verify_locations(ctx,"control/clientca.pem",NULL);

  /* a new SSL object, with the rest added to it directly to avoid copying */
  if (!(ssl = SSL_new(ctx))) die_read();
  SSL_CTX_free(ctx);
  if (!SSL_use_RSAPrivateKey_file(ssl,"control/servercert.pem",SSL_FILETYPE_PEM)) {
    out("454 TLS not available: missing RSA private key (#4.3.0)\r\n"); 
    flush();
    if (smtps) die_read();
    return;
  }
  SSL_set_tmp_rsa_callback(ssl,tmp_rsa_cb);
  SSL_set_cipher_list(ssl,tlsserverciphers.s);
  SSL_set_verify(ssl,SSL_VERIFY_NONE,verify_cb);
 
  if (!smtps) { out("220 ready for tls\r\n"); flush(); }

  ssl_rfd = substdio_fileno(&ssin);
  SSL_set_rfd(ssl,ssl_rfd);
  ssl_wfd = substdio_fileno(&ssout);
  SSL_set_wfd(ssl,ssl_wfd);

  if (ssl_timeoutaccept(timeout,ssl_rfd,ssl_wfd,ssl) <= 0) {
    SSL_free(ssl); ssl = 0; /* so that out() doesn't go via SSL_write() */
    /* cleartext response is not part of any standard, nor is any other response here */
    out("454 TLS not available: connection ");
    if (errno == error_timeout)
      out("timed out");
    else
      out("failed");
    out(" (#4.3.0)\r\n");
    flush();
    die_read();
  }

  /* have to discard the pre-STARTTLS HELO/EHLO argument, if any */
  dohelo(remotehost);
}
#endif

struct commands smtpcommands[] = {
  { "rcpt", smtp_rcpt, 0 }
, { "mail", smtp_mail, 0 }
, { "data", smtp_data, flush }
, { "quit", smtp_quit, flush }
, { "helo", smtp_helo, flush }
, { "ehlo", smtp_ehlo, flush }
, { "rset", smtp_rset, 0 }
, { "help", smtp_help, flush }
#ifdef TLS
, { "starttls", smtp_tls, flush }
#endif
, { "noop", err_noop, flush }
, { "vrfy", err_vrfy, flush }
, { 0, err_unimpl, flush }
} ;

void main()
{
  sig_pipeignore();
  if (chdir(auto_qmail) == -1) die_control();
  setup();
  if (ipme_init() != 1) die_ipme();
  smtp_greet("220 ");
  out(" ESMTP\r\n");
  if (commands(&ssin,&smtpcommands) == 0) die_read();
  die_nomem();
}
