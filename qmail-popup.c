#include <unistd.h>
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
#include "noreturn.h"
#include "readwrite.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

void _noreturn_ die() { _exit(1); }

GEN_SAFE_TIMEOUTREAD(saferead,1200,fd,die())
GEN_SAFE_TIMEOUTWRITE(safewrite,1200,fd,die())

char ssoutbuf[128];
substdio ssout = SUBSTDIO_FDBUF(safewrite,1,ssoutbuf,sizeof(ssoutbuf));

char ssinbuf[128];
substdio ssin = SUBSTDIO_FDBUF(saferead,0,ssinbuf,sizeof(ssinbuf));

void puts(char *s)
{
  substdio_puts(&ssout,s);
}
void flush(void)
{
  substdio_flush(&ssout);
}
void err(char *s)
{
  puts("-ERR ");
  puts(s);
  puts("\r\n");
  flush();
}

void _noreturn_ die_usage(void) { err("usage: popup hostname subprogram"); die(); }
void _noreturn_ die_nomem(void) { err("out of memory"); die(); }
void _noreturn_ die_pipe(void) { err("unable to open pipe"); die(); }
void _noreturn_ die_write(void) { err("unable to write pipe"); die(); }
void _noreturn_ die_fork(void) { err("unable to fork"); die(); }
void die_childcrashed(void) { err("aack, child crashed"); }
void die_badauth(void) { err("authorization failed"); }

void err_syntax(void) { err("syntax error"); }
void err_wantuser(void) { err("USER first"); }
void err_authoriz(char *arg, void *data) { err("authorization first"); }

void okay(char *arg, void *data) { puts("+OK \r\n"); flush(); }
void _noreturn_ pop3_quit(char *arg, void *data) { okay(0, 0); die(); }


char unique[FMT_ULONG + FMT_ULONG + 3];
char *hostname;
stralloc username = {0};
int seenuser = 0;
char **childargs;
substdio ssup;
char upbuf[128];


void _noreturn_ doanddie(char *user,
                         unsigned int userlen, /* including 0 byte */
                         char *pass)
{
  int child;
  int wstat;
  int pi[2];
 
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
  substdio_fdbuf(&ssup,write,pi[1],upbuf,sizeof(upbuf));
  if (substdio_put(&ssup,user,userlen) == -1) die_write();
  if (substdio_put(&ssup,pass,str_len(pass) + 1) == -1) die_write();
  if (substdio_puts(&ssup,"<") == -1) die_write();
  if (substdio_puts(&ssup,unique) == -1) die_write();
  if (substdio_puts(&ssup,hostname) == -1) die_write();
  if (substdio_put(&ssup,">",2) == -1) die_write();
  if (substdio_flush(&ssup) == -1) die_write();
  close(pi[1]);
  byte_zero(pass,str_len(pass));
  byte_zero(upbuf,sizeof(upbuf));
  if (wait_pid(&wstat,child) == -1) die();
  if (wait_crashed(wstat)) die_childcrashed();
  if (wait_exitcode(wstat)) die_badauth();
  die();
}
void pop3_greet(void)
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
void pop3_user(char *arg, void *data)
{
  if (!*arg) { err_syntax(); return; }
  okay(0, 0);
  seenuser = 1;
  if (!stralloc_copys(&username,arg)) die_nomem(); 
  if (!stralloc_0(&username)) die_nomem(); 
}
void pop3_pass(char *arg, void *data)
{
  if (!seenuser) { err_wantuser(); return; }
  if (!*arg) { err_syntax(); return; }
  doanddie(username.s,username.len,arg);
}
void pop3_apop(char *arg, void *apop)
{
  char *space;
  space = arg + str_chr(arg,' ');
  if (!*space) { err_syntax(); return; }
  *space++ = 0;
  doanddie(arg,space - arg,space);
}

struct command pop3commands[] = {
  { "user", pop3_user, 0, 0 }
, { "pass", pop3_pass, 0, 0 }
, { "apop", pop3_apop, 0, 0 }
, { "quit", pop3_quit, 0, 0 }
, { "noop", okay, 0, 0 }
, { 0, err_authoriz, 0, 0 }
} ;

int main(int argc, char **argv)
{
  stralloc line = {0};
  sig_alarmcatch(die);
  sig_pipeignore();
 
  hostname = argv[1];
  if (!hostname) die_usage();
  childargs = argv + 2;
  if (!*childargs) die_usage();
 
  pop3_greet();
  for (;;) {
    line.len = 0;

    for (;;) {
      int j;
      if (!stralloc_readyplus(&line, 1))
        die_nomem();
      j = substdio_get(&ssin, line.s + line.len, 1);
      if (j == 0)
        die();
      if (line.s[line.len] == '\n') break;
      ++line.len;
    }
    execute_command(line.s, line.len, pop3commands);
  }
}
