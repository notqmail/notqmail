#include <sys/types.h>
#include <sys/stat.h>
#include "fd.h"
#include "sig.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "alloc.h"
#include "datetime.h"
#include "error.h"
#include "wait.h"
#include "str.h"
#include "now.h"
#include "fmt.h"
#include "exit.h"
#include "readwrite.h"

int timeout = 1200;

int timeoutread(fd,buf,n) int fd; char *buf; int n;
{
 int r; int saveerrno;
 alarm(timeout);
 r = read(fd,buf,n); saveerrno = errno;
 alarm(0);
 errno = saveerrno; return r;
}

char ssinbuf[128];
substdio ssin = SUBSTDIO_FDBUF(timeoutread,0,ssinbuf,sizeof(ssinbuf));


void die() { _exit(1); }
void out(s) char *s;
{
 if (substdio_puts(subfdoutsmall,s) == -1) die();
}
void outflush(s) char *s; 
{
 out(s);
 if (substdio_flush(subfdoutsmall) == -1) die();
}
void err(s) char *s;
{
 if (substdio_puts(subfdoutsmall,"-ERR ") == -1) die();
 if (substdio_puts(subfdoutsmall,s) == -1) die();
 if (substdio_puts(subfdoutsmall,"\r\n") == -1) die();
 if (substdio_flush(subfdoutsmall) == -1) die();
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

void okay() { outflush("+OK \r\n"); }


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
 int i;

 if (fd_copy(2,1) == -1) die_pipe();
 close(3);
 if (pipe(pi) == -1) die_pipe();
 if (pi[0] != 3) die_pipe();
 switch(child = fork())
  {
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
 for (i = 0;pass[i];++i) pass[i] = 0;
 for (i = 0;i < sizeof(upbuf);++i) upbuf[i] = 0;
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

 out("+OK <");
 out(unique);
 out(hostname);
 outflush(">\r\n");
}
void pop3_user(arg) char *arg;
{
 if (!arg) { err_syntax(); return; }
 okay();
 seenuser = 1;
 if (!stralloc_copys(&username,arg)) die_nomem(); 
 if (!stralloc_0(&username)) die_nomem(); 
}
void pop3_pass(arg) char *arg;
{
 if (!seenuser) { err_wantuser(); return; }
 if (!arg) { err_syntax(); return; }
 doanddie(username.s,username.len,arg);
}
void pop3_apop(arg) char *arg;
{
 char *space;
 if (!arg) { err_syntax(); return; }
 space = arg + str_chr(arg,' ');
 if (!*space) { err_syntax(); return; }
 *space++ = 0;
 doanddie(arg,space - arg,space);
}

void pop3_quit() { okay(); die(); }

static struct { void (*fun)(); char *text; } pop3cmd[] = {
  { pop3_user, "user" }
, { pop3_pass, "pass" }
, { pop3_apop, "apop" }
, { pop3_quit, "quit" }
, { okay, "noop" }
, { 0, 0 }
};

void doit(cmd)
char *cmd;
{
 int i;
 int j;
 char ch;

 for (i = 0;pop3cmd[i].fun;++i)
  {
   for (j = 0;ch = pop3cmd[i].text[j];++j)
     if ((cmd[j] != ch) && (cmd[j] != ch - 32))
       break;
   if (!ch)
     if (!cmd[j] || (cmd[j] == ' '))
      {
       while (cmd[j] == ' ') ++j;
       if (!cmd[j])
         pop3cmd[i].fun((char *) 0);
       else
         pop3cmd[i].fun(cmd + j);
       return;
      }
  }
 err_authoriz();
}

void main(argc,argv)
int argc;
char **argv;
{
 static stralloc cmd = {0};
 int match;

 sig_alarmcatch(die);
 sig_pipeignore();

 hostname = argv[1];
 if (!hostname) die_usage();
 childargs = argv + 2;
 if (!*childargs) die_usage();

 pop3_greet();

 for (;;)
  {
   if (getln(&ssin,&cmd,&match,'\n') == -1) die();
   if (!match) die();
   if (cmd.len == 0) die();
   if (cmd.s[--cmd.len] != '\n') die();
   if ((cmd.len > 0) && (cmd.s[cmd.len - 1] == '\r')) --cmd.len;
   cmd.s[cmd.len++] = 0;
   doit(cmd.s);
  }
}
