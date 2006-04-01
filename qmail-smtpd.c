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
#include "wait.h"
#include "qmail-spp.h"

#define AUTHSLEEP 5

int spp_val;

#define MAXHOPS 100
unsigned int databytes = 0;
int timeout = 1200;

int safewrite(fd,buf,len) int fd; char *buf; int len;
{
  int r;
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
void straynewline() { out("451 See https://cr.yp.to/docs/smtplf.html.\r\n"); flush(); _exit(1); }

void err_size() { out("552 sorry, that message size exceeds my databytes limit (#5.3.4)\r\n"); }
void err_bmf() { out("553 sorry, your envelope sender is in my badmailfrom list (#5.7.1)\r\n"); }
void err_nogateway() { out("553 sorry, that domain isn't in my list of allowed rcpthosts (#5.7.1)\r\n"); }
void err_unimpl(arg) char *arg; { out("502 unimplemented (#5.5.1)\r\n"); }
void err_syntax() { out("555 syntax error (#5.5.4)\r\n"); }
void err_wantmail() { out("503 MAIL first (#5.5.1)\r\n"); }
void err_wantrcpt() { out("503 RCPT first (#5.5.1)\r\n"); }
void err_noop(arg) char *arg; { out("250 ok\r\n"); }
void err_vrfy(arg) char *arg; { out("252 send some mail, i'll try my best\r\n"); }
void err_qqt() { out("451 qqt failure (#4.3.0)\r\n"); }

int err_child() { out("454 oops, problem with child and I can't auth (#4.3.0)\r\n"); return -1; }
int err_fork() { out("454 oops, child won't start and I can't auth (#4.3.0)\r\n"); return -1; }
int err_pipe() { out("454 oops, unable to open pipe and I can't auth (#4.3.0)\r\n"); return -1; }
int err_write() { out("454 oops, unable to write pipe and I can't auth (#4.3.0)\r\n"); return -1; }
void err_authd() { out("503 you're already authenticated (#5.5.0)\r\n"); }
void err_authmail() { out("503 no auth during mail transaction (#5.5.0)\r\n"); }
int err_noauth() { out("504 auth type unimplemented (#5.5.1)\r\n"); return -1; }
int err_authabrt() { out("501 auth exchange canceled (#5.0.0)\r\n"); return -1; }
int err_input() { out("501 malformed auth input (#5.5.4)\r\n"); return -1; }
void err_authfail() { out("535 authentication failed (#5.7.1)\r\n"); }
void err_submission() { out("530 Authorization required (#5.7.1) \r\n"); }

stralloc greeting = {0};

void smtp_greet(code) char *code;
{
  substdio_puts(&ssout,code);
  substdio_put(&ssout,greeting.s,greeting.len);
}
void smtp_help(arg) char *arg;
{
  out("214 notqmail home page: https://notqmail.org\r\n");
}
void smtp_quit(arg) char *arg;
{
  smtp_greet("221 "); out("\r\n"); flush(); _exit(0);
}

char *protocol;
char *remoteip;
char *remotehost;
char *remoteinfo;
char *local;
char *localport;
char *relayclient;
char *auth;

stralloc helohost = {0};
char *fakehelo; /* pointer into helohost, or 0 */

void dohelo(arg) char *arg; {
  if (!stralloc_copys(&helohost,arg)) die_nomem(); 
  if (!stralloc_0(&helohost)) die_nomem(); 
  fakehelo = case_diffs(remotehost,helohost.s) ? helohost.s : 0;
}

int smtpauth = 0;
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
  if (spp_init() == -1) die_control();

  bmfok = control_readfile(&bmf,"control/badmailfrom",0);
  if (bmfok == -1) die_control();
  if (bmfok)
    if (!constmap_init(&mapbmf,bmf.s,bmf.len,0)) die_nomem();
 
  if (control_readint(&databytes,"control/databytes") == -1) die_control();
  x = env_get("DATABYTES");
  if (x) { scan_ulong(x,&u); databytes = u; }
  if (!(databytes + 1)) --databytes;
 
  protocol = "SMTP";
  remoteip = env_get("TCPREMOTEIP");
  if (!remoteip) remoteip = "unknown";
  local = env_get("TCPLOCALHOST");
  if (!local) local = env_get("TCPLOCALIP");
  if (!local) local = "unknown";
  localport = env_get("TCPLOCALPORT");
  if (!localport) localport = "0";
  remotehost = env_get("TCPREMOTEHOST");
  if (!remotehost) remotehost = "unknown";
  remoteinfo = env_get("TCPREMOTEINFO");
  relayclient = env_get("RELAYCLIENT");
  auth = env_get("SMTPAUTH");
  if (auth) {
    smtpauth = 1;
    case_lowers(auth);
    if (!case_diffs(auth,"-")) smtpauth = 0;
    if (!case_diffs(auth,"!")) smtpauth = 11;
    if (case_starts(auth,"cram")) smtpauth = 2;
    if (case_starts(auth,"+cram")) smtpauth = 3;
    if (case_starts(auth,"!cram")) smtpauth = 12;
    if (case_starts(auth,"!+cram")) smtpauth = 13;
  }
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

char *auth;
int seenauth = 0;
int seenmail = 0;
int flagbarf; /* defined if seenmail */
int flagsize;
int allowed;
stralloc mailfrom = {0};
stralloc rcptto = {0};
stralloc fuser = {0};
stralloc mfparms = {0};

int mailfrom_size(arg) char *arg;
{
  long r;
  unsigned long sizebytes = 0;

  scan_ulong(arg,&r);
  sizebytes = r;
  if (databytes) if (sizebytes > databytes) return 1;
  return 0;
}

void mailfrom_auth(arg,len) 
char *arg; 
int len;
{
  if (!stralloc_copys(&fuser,"")) die_nomem();
  if (case_starts(arg,"<>")) { if (!stralloc_cats(&fuser,"unknown")) die_nomem(); }
  else 
    while (len) {
      if (*arg == '+') {
        if (case_starts(arg,"+3D")) { arg=arg+2; len=len-2; if (!stralloc_cats(&fuser,"=")) die_nomem(); }
        if (case_starts(arg,"+2B")) { arg=arg+2; len=len-2; if (!stralloc_cats(&fuser,"+")) die_nomem(); }
      }
      else
        if (!stralloc_catb(&fuser,arg,1)) die_nomem();
      arg++; len--;
    }
  if(!stralloc_0(&fuser)) die_nomem();
  if (!remoteinfo) {
    remoteinfo = fuser.s;
    if (!env_unset("TCPREMOTEINFO")) die_read();
    if (!env_put2("TCPREMOTEINFO",remoteinfo)) die_nomem();
  }
}

void mailfrom_parms(arg) char *arg;
{
  int i;
  int len;

    len = str_len(arg);
    if (!stralloc_copys(&mfparms,"")) die_nomem();
    i = byte_chr(arg,len,'>');
    if (i > 4 && i < len) {
      while (len) {
        arg++; len--; 
        if (*arg == ' ' || *arg == '\0' ) {
           if (case_starts(mfparms.s,"SIZE=")) if (mailfrom_size(mfparms.s+5)) { flagsize = 1; return; }
           if (case_starts(mfparms.s,"AUTH=")) mailfrom_auth(mfparms.s+5,mfparms.len-5);  
           if (!stralloc_copys(&mfparms,"")) die_nomem();
        }
        else
          if (!stralloc_catb(&mfparms,arg,1)) die_nomem(); 
      }
    }
}

void smtp_helo(arg) char *arg;
{
  if(!spp_helo(arg)) return;
  smtp_greet("250 "); out("\r\n");
  seenmail = 0; dohelo(arg);
}
void smtp_ehlo(arg) char *arg;
{
  char size[FMT_ULONG];
  if(!spp_helo(arg)) return;
  size[fmt_ulong(size,(unsigned int) databytes)] = 0;
  smtp_greet("250-");
  out("\r\n250-PIPELINING\r\n250-8BITMIME\r\n");
  if (smtpauth == 1 || smtpauth == 11) out("250-AUTH LOGIN PLAIN\r\n");
  if (smtpauth == 2 || smtpauth == 12) out("250-AUTH CRAM-MD5\r\n");
  if (smtpauth == 3 || smtpauth == 13) out("250-AUTH LOGIN PLAIN CRAM-MD5\r\n");
  out("250 SIZE "); out(size); out("\r\n");
  seenmail = 0; dohelo(arg);
}
void smtp_rset(arg) char *arg;
{
  spp_rset();
  seenmail = 0; seenauth = 0;
  mailfrom.len = 0; rcptto.len = 0;
  out("250 flushed\r\n");
}
void smtp_mail(arg) char *arg;
{
  if (smtpauth)
    if (smtpauth > 10 && !seenauth) { err_submission(); return; }
  if (!addrparse(arg)) { err_syntax(); return; }
  if (!(spp_val = spp_mail())) return;
  flagsize = 0;
  mailfrom_parms(arg);
  if (flagsize) { err_size(); return; }
  if (spp_val == 1)
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
  if (!relayclient) allowed = addrallowed();
  else allowed = 1;
  if (!(spp_val = spp_rcpt(allowed))) return;
  if (relayclient) {
    --addr.len;
    if (!stralloc_cats(&addr,relayclient)) die_nomem();
    if (!stralloc_0(&addr)) die_nomem();
  }
  else if (spp_val == 1) {
    if (!allowed) { err_nogateway(); return; }
  }
  spp_rcpt_accepted();
  if (!stralloc_cats(&rcptto,"T")) die_nomem();
  if (!stralloc_cats(&rcptto,addr.s)) die_nomem();
  if (!stralloc_0(&rcptto)) die_nomem();
  out("250 ok\r\n");
}


int saferead(fd,buf,len) int fd; char *buf; int len;
{
  int r;
  flush();
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
	++pos;
      }
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

void smtp_data(arg) char *arg; {
  int hops;
  unsigned long qp;
  char *qqx;
 
  if (!seenmail) { err_wantmail(); return; }
  if (!rcptto.len) { err_wantrcpt(); return; }
  if (!spp_data()) return;
  seenmail = 0;
  if (databytes) bytestooverflow = databytes + 1;
  if (qmail_open(&qqt) == -1) { err_qqt(); return; }
  qp = qmail_qp(&qqt);
  out("354 go ahead\r\n");
 
  received(&qqt,protocol,local,remoteip,remotehost,remoteinfo,fakehelo);
  qmail_put(&qqt,sppheaders.s,sppheaders.len); /* set in qmail-spp.c */
  spp_rset();
  blast(&hops);
  hops = (hops >= MAXHOPS);
  if (hops) qmail_fail(&qqt);
  qmail_from(&qqt,mailfrom.s);
  qmail_put(&qqt,rcptto.s,rcptto.len);
 
  qqx = qmail_close(&qqt);
  if (!*qqx) { acceptmessage(qp); return; }
  if (hops) { out("554 too many hops, this message is looping (#5.4.6)\r\n"); return; }
  if (databytes) if (!bytestooverflow) { err_size(); return; }
  if (*qqx == 'D') out("554 "); else out("451 ");
  out(qqx + 1);
  out("\r\n");
}

/* this file is too long ----------------------------------------- SMTP AUTH */

char unique[FMT_ULONG + FMT_ULONG + 3];
static stralloc authin = {0};   /* input from SMTP client */
static stralloc user = {0};     /* authorization user-id */
static stralloc pass = {0};     /* plain passwd or digest */
static stralloc resp = {0};     /* b64 response */
static stralloc chal = {0};     /* plain challenge */
static stralloc slop = {0};     /* b64 challenge */

char **childargs;
char ssauthbuf[512];
substdio ssauth = SUBSTDIO_FDBUF(safewrite,3,ssauthbuf,sizeof(ssauthbuf));

int authgetl(void) {
  int i;

  if (!stralloc_copys(&authin,"")) die_nomem();
  for (;;) {
    if (!stralloc_readyplus(&authin,1)) die_nomem(); /* XXX */
    i = substdio_get(&ssin,authin.s + authin.len,1);
    if (i != 1) die_read();
    if (authin.s[authin.len] == '\n') break;
    ++authin.len;
  }

  if (authin.len > 0) if (authin.s[authin.len - 1] == '\r') --authin.len;
  authin.s[authin.len] = 0;
  if (*authin.s == '*' && *(authin.s + 1) == 0) { return err_authabrt(); }
  if (authin.len == 0) { return err_input(); }
  return authin.len;
}

int authenticate(void)
{
  int child;
  int wstat;
  int pi[2];

  if (!stralloc_0(&user)) die_nomem();
  if (!stralloc_0(&pass)) die_nomem();
  if (!stralloc_0(&chal)) die_nomem();

  if (pipe(pi) == -1) return err_pipe();
  switch(child = fork()) {
    case -1:
      return err_fork();
    case 0:
      close(pi[1]);
      if(fd_copy(3,pi[0]) == -1) return err_pipe();
      sig_pipedefault();
        execvp(*childargs, childargs);
      _exit(1);
  }
  close(pi[0]);

  substdio_fdbuf(&ssauth,write,pi[1],ssauthbuf,sizeof ssauthbuf);
  if (substdio_put(&ssauth,user.s,user.len) == -1) return err_write();
  if (substdio_put(&ssauth,pass.s,pass.len) == -1) return err_write();
  if (smtpauth == 2 || smtpauth == 3 || smtpauth == 12 || smtpauth == 13)  
    if (substdio_put(&ssauth,chal.s,chal.len) == -1) return err_write();
  if (substdio_flush(&ssauth) == -1) return err_write();

  close(pi[1]);
  if (!stralloc_copys(&chal,"")) die_nomem();
  if (!stralloc_copys(&slop,"")) die_nomem();
  byte_zero(ssauthbuf,sizeof ssauthbuf);
  if (wait_pid(&wstat,child) == -1) return err_child();
  if (wait_crashed(wstat)) return err_child();
  if (wait_exitcode(wstat)) { sleep(AUTHSLEEP); return 1; } /* no */
  return 0; /* yes */
}

int auth_login(arg) char *arg;
{
  int r;

  if (*arg) {
    if (r = b64decode(arg,str_len(arg),&user) == 1) return err_input();
  }
  else {
    out("334 VXNlcm5hbWU6\r\n"); flush();       /* Username: */
    if (authgetl() < 0) return -1;
    if (r = b64decode(authin.s,authin.len,&user) == 1) return err_input();
  }
  if (r == -1) die_nomem();

  out("334 UGFzc3dvcmQ6\r\n"); flush();         /* Password: */

  if (authgetl() < 0) return -1;
  if (r = b64decode(authin.s,authin.len,&pass) == 1) return err_input();
  if (r == -1) die_nomem();

  if (!user.len || !pass.len) return err_input();
  return authenticate();
}

int auth_plain(arg) char *arg;
{
  int r, id = 0;

  if (*arg) {
    if (r = b64decode(arg,str_len(arg),&resp) == 1) return err_input();
  }
  else {
    out("334 \r\n"); flush();
    if (authgetl() < 0) return -1;
    if (r = b64decode(authin.s,authin.len,&resp) == 1) return err_input();
  }
  if (r == -1 || !stralloc_0(&resp)) die_nomem();
  while (resp.s[id]) id++;                       /* "authorize-id\0userid\0passwd\0" */

  if (resp.len > id + 1)
    if (!stralloc_copys(&user,resp.s + id + 1)) die_nomem();
  if (resp.len > id + user.len + 2)
    if (!stralloc_copys(&pass,resp.s + id + user.len + 2)) die_nomem();

  if (!user.len || !pass.len) return err_input();
  return authenticate();
}

int auth_cram()
{
  int i, r;
  char *s;

  s = unique;                                           /* generate challenge */
  s += fmt_uint(s,getpid());
  *s++ = '.';
  s += fmt_ulong(s,(unsigned long) now());
  *s++ = '@';
  *s++ = 0;
  if (!stralloc_copys(&chal,"<")) die_nomem();
  if (!stralloc_cats(&chal,unique)) die_nomem();
  if (!stralloc_cats(&chal,local)) die_nomem();
  if (!stralloc_cats(&chal,">")) die_nomem();
  if (b64encode(&chal,&slop) < 0) die_nomem();
  if (!stralloc_0(&slop)) die_nomem();

  out("334 ");                                          /* "334 base64_challenge \r\n" */
  out(slop.s);
  out("\r\n");
  flush();

  if (authgetl() < 0) return -1;                        /* got response */
  if (r = b64decode(authin.s,authin.len,&resp) == 1) return err_input();
  if (r == -1 || !stralloc_0(&resp)) die_nomem();

  i = str_rchr(resp.s,' ');
  s = resp.s + i;
  while (*s == ' ') ++s;
  resp.s[i] = 0;
  if (!stralloc_copys(&user,resp.s)) die_nomem();       /* userid */
  if (!stralloc_copys(&pass,s)) die_nomem();            /* digest */

  if (!user.len || !pass.len) return err_input();
  return authenticate();
}

struct authcmd {
  char *text;
  int (*fun)();
} authcmds[] = {
  { "login",auth_login }
, { "plain",auth_plain }
, { "cram-md5",auth_cram }
, { 0,err_noauth }
};

void smtp_auth(arg)
char *arg;
{
  int i;
  char *cmd = arg;

  if (!smtpauth || !*childargs) { out("503 auth not available (#5.3.3)\r\n"); return; }
  if (seenauth) { err_authd(); return; }
  if (seenmail) { err_authmail(); return; }

  if (!stralloc_copys(&user,"")) die_nomem();
  if (!stralloc_copys(&pass,"")) die_nomem();
  if (!stralloc_copys(&resp,"")) die_nomem();
  if (!stralloc_copys(&chal,"")) die_nomem();

  i = str_chr(cmd,' ');
  arg = cmd + i;
  while (*arg == ' ') ++arg;
  cmd[i] = 0;

  for (i = 0;authcmds[i].text;++i)
    if (case_equals(authcmds[i].text,cmd)) break;

  switch (authcmds[i].fun(arg)) {
    case 0:
      seenauth = 1;
      protocol = "ESMTPA";
      relayclient = "";
      remoteinfo = user.s;
      if (!env_unset("TCPREMOTEINFO")) die_read();
      if (!env_put2("TCPREMOTEINFO",remoteinfo)) die_nomem();
      if (!env_put2("RELAYCLIENT",relayclient)) die_nomem();
      out("235 ok, go ahead (#2.0.0)\r\n");
      break;
    case 1:
      err_authfail(user.s,authcmds[i].text);
  }
}


/* this file is too long --------------------------------------------- GO ON */

struct commands smtpcommands[] = {
  { "rcpt", smtp_rcpt, 0 }
, { "mail", smtp_mail, 0 }
, { "data", smtp_data, flush }
, { "auth", smtp_auth, flush }
, { "quit", smtp_quit, flush }
, { "helo", smtp_helo, flush }
, { "ehlo", smtp_ehlo, flush }
, { "rset", smtp_rset, 0 }
, { "help", smtp_help, flush }
, { "noop", err_noop, flush }
, { "vrfy", err_vrfy, flush }
, { 0, err_unimpl, flush }
} ;

void main(argc,argv)
int argc;
char **argv;
{
  childargs = argv + 1;
  sig_pipeignore();
  if (chdir(auto_qmail) == -1) die_control();
  setup();
  if (ipme_init() != 1) die_ipme();
  if (spp_connect()) {
  smtp_greet("220 ");
  out(" ESMTP\r\n");
  }
  if (commands(&ssin,&smtpcommands) == 0) die_read();
  die_nomem();
}
