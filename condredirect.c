#include "sig.h"
#include "readwrite.h"
#include "exit.h"
#include "env.h"
#include "error.h"
#include "fork.h"
#include "wait.h"
#include "seek.h"
#include "qmail.h"
#include "stralloc.h"
#include "subfd.h"
#include "substdio.h"

void die_success() { _exit(0); }
void die_99() { _exit(99); }
void die_perm(s) char *s; { substdio_putsflush(subfderr,s); _exit(100); }
void die_temp(s) char *s; { substdio_putsflush(subfderr,s); _exit(111); }
void die_nomem() { die_temp("condredirect: fatal: out of memory\n"); }

struct qmail qqt;

int mywrite(fd,buf,len) int fd; char *buf; int len;
{
 qmail_put(&qqt,buf,len);
 return len;
}

substdio ssin;
substdio ssout;
char inbuf[SUBSTDIO_INSIZE];
char outbuf[16];

void main(argc,argv)
int argc;
char **argv;
{
 char *sender;
 char *dtline;
 int pid;
 int wstat;

 if (!argv[1] || !argv[2])
   die_perm("condredirect: usage: condredirect newaddress program arg ...\n");

 switch(pid = fork())
  {
   case -1: die_temp("condredirect: fatal: unable to fork\n");
   case 0:
     execvp(argv[2],argv + 2);
     if (error_temp(errno)) _exit(111);
     _exit(100);
  }
 if (wait_pid(&wstat,pid) != pid)
   die_perm("condredirect: fatal: internal bug\n");
 if (wait_crashed(wstat)) die_temp("condredirect: fatal: child crashed\n");
 switch(wait_exitcode(wstat))
  {
   case 0: break;
   case 111: die_temp("condredirect: fatal: temporary child error\n");
   default: die_success();
  }

 if (seek_begin(0) == -1) die_temp("condredirect: fatal: unable to rewind\n");
 sig_pipeignore();

 sender = env_get("SENDER");
 if (!sender) die_perm("condredirect: fatal: SENDER not set\n");
 dtline = env_get("DTLINE");
 if (!dtline) die_perm("condredirect: fatal: DTLINE not set\n");

 if (qmail_open(&qqt) == -1) die_temp("condredirect: fatal: unable to fork\n");
 qmail_puts(&qqt,dtline);
 substdio_fdbuf(&ssin,read,0,inbuf,sizeof(inbuf));
 substdio_fdbuf(&ssout,mywrite,-1,outbuf,sizeof(outbuf));
 if (substdio_copy(&ssout,&ssin) != 0)
   die_temp("condredirect: fatal: error while reading message\n");
 substdio_flush(&ssout);

 qmail_from(&qqt,sender);
 qmail_to(&qqt,argv[1]);
 switch(qmail_close(&qqt))
  {
   case 0: die_99();
   case QMAIL_TOOLONG: die_perm("condredirect: fatal: permanent qmail-queue error\n");
   default: die_temp("condredirect: fatal: temporary qmail-queue error\n");
  }
}
