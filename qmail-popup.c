#include "commands.h"
#include "fd.h"
#include "sig.h"
#include "stralloc.h"
#include "substdio.h"
#include "alloc.h"
#include "wait.h"
#include "str.h"
#include "byte.h"
#include "now.h"
#include "fmt.h"
#include "exit.h"
#include "readwrite.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

void die() { _exit(1); }

int saferead(fd,buf,len) int fd; char *buf; int len;
{
  int r;
  r = timeoutread(1200,fd,buf,len);
  if (r <= 0) die();
  return r;
}

int safewrite(fd,buf,len) int fd; char *buf; int len;
{
  int r;
  r = timeoutwrite(1200,fd,buf,len);
  if (r <= 0) die();
  return r;
}

char ssoutbuf[128];
substdio ssout = SUBSTDIO_FDBUF(safewrite,1,ssoutbuf,sizeof ssoutbuf);

char ssinbuf[128];
substdio ssin = SUBSTDIO_FDBUF(saferead,0,ssinbuf,sizeof ssinbuf);

void puts(s) char *s;
{
  substdio_puts(&ssout,s);
}
void flush()
{
  substdio_flush(&ssout);
}
void err(s) char *s;
{
  puts("-ERR ");
  puts(s);
  puts("\r\n");
  flush();
}

void die_usage() { err("usage: popup hostname subprogram"); die(); }
void die_nomem() { err("out of memory"); die(); }
void die_pipe() { err("unable to open pipe"); die(); }
void die_write() { err("unable to write pipe"); die(); }
void die_fork() { err("unable to fork"); die(); }
void die_childcrashed() { err("aack, child crashed"); }
void die_badauth() { err("authorization failed"); }

void err_syntax() { err("syntax error"); }
void err_wantuser() { err("USER first"); }
void err_authoriz() { err("authorization first"); }

void okay() { puts("+OK \r\n"); flush(); }
void pop3_quit() { okay(); die(); }


char unique[FMT_ULONG + FMT_ULONG + 3];
char *hostname;
stralloc username = {0};
int seenuser = 0;
char **childargs;
substdio ssup;
char upbuf[128];


void doanddie(user,userlen,pass)
char *user;
unsigned int userlen; /* including 0 byte */
char *pass;
{
  int child;
  int wstat;
  int pi[2];
 
  if (fd_copy(2,1) == -1) die_pipe();
  close(3);
  if (pipe(pi) == -1) die_pipe();
  if (pi[0] != 3) die_pipe();
  switch(child = fork()) {
    case -1:
      die_fork();
    case 0:
      close(pi[1]);
      sig_pipedefault();
      execvp(*childargs,childargs);
      _exit(1);
  }
  close(pi[0]);
  substdio_fdbuf(&ssup,write,pi[1],upbuf,sizeof upbuf);
  if (substdio_put(&ssup,user,userlen) == -1) die_write();
  if (substdio_put(&ssup,pass,str_len(pass) + 1) == -1) die_write();
  if (substdio_puts(&ssup,"<") == -1) die_write();
  if (substdio_puts(&ssup,unique) == -1) die_write();
  if (substdio_puts(&ssup,hostname) == -1) die_write();
  if (substdio_put(&ssup,">",2) == -1) die_write();
  if (substdio_flush(&ssup) == -1) die_write();
  close(pi[1]);
  byte_zero(pass,str_len(pass));
  byte_zero(upbuf,sizeof upbuf);
  if (wait_pid(&wstat,child) == -1) die();
  if (wait_crashed(wstat)) die_childcrashed();
  if (wait_exitcode(wstat)) die_badauth();
  die();
}
void pop3_greet()
{
  char *s;
  s = unique;
  s += fmt_uint(s,getpid());
  *s++ = '.';
  s += fmt_ulong(s,(unsigned long) now());
  *s++ = '@';
  *s++ = 0;
  puts("+OK <");
  puts(unique);
  puts(hostname);
  puts(">\r\n");
  flush();
}
void pop3_user(arg) char *arg;
{
  if (!*arg) { err_syntax(); return; }
  okay();
  seenuser = 1;
  if (!stralloc_copys(&username,arg)) die_nomem(); 
  if (!stralloc_0(&username)) die_nomem(); 
}
void pop3_pass(arg) char *arg;
{
  if (!seenuser) { err_wantuser(); return; }
  if (!*arg) { err_syntax(); return; }
  doanddie(username.s,username.len,arg);
}
void pop3_apop(arg) char *arg;
{
  char *space;
  space = arg + str_chr(arg,' ');
  if (!*space) { err_syntax(); return; }
  *space++ = 0;
  doanddie(arg,space - arg,space);
}

struct commands pop3commands[] = {
  { "user", pop3_user, 0 }
, { "pass", pop3_pass, 0 }
, { "apop", pop3_apop, 0 }
, { "quit", pop3_quit, 0 }
, { "noop", okay, 0 }
, { 0, err_authoriz, 0 }
} ;

void main(argc,argv)
int argc;
char **argv;
{
  sig_alarmcatch(die);
  sig_pipeignore();
 
  hostname = argv[1];
  if (!hostname) die_usage();
  childargs = argv + 2;
  if (!*childargs) die_usage();
 
  pop3_greet();
  commands(&ssin,pop3commands);
  die();
}
