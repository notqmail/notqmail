#include "sig.h"
#include "readwrite.h"
#include "exit.h"
#include "env.h"
#include "error.h"
#include "fork.h"
#include "wait.h"
#include "seek.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"

#define FATAL "condredirect: fatal: "

struct qmail qqt;

int mywrite(fd,buf,len) int fd; char *buf; int len;
{
  qmail_put(&qqt,buf,len);
  return len;
}

char inbuf[SUBSTDIO_INSIZE];
char outbuf[1];
substdio ssin = SUBSTDIO_FDBUF(read,0,inbuf,sizeof inbuf);
substdio ssout = SUBSTDIO_FDBUF(mywrite,-1,outbuf,sizeof outbuf);

char num[FMT_ULONG];

void main(argc,argv)
int argc;
char **argv;
{
  char *sender;
  char *dtline;
  int pid;
  int wstat;
  char *qqx;
 
  if (!argv[1] || !argv[2])
    strerr_die1x(100,"condredirect: usage: condredirect newaddress program [ arg ... ]");
 
  pid = fork();
  if (pid == -1)
    strerr_die2sys(111,FATAL,"unable to fork: ");
  if (pid == 0) {
    execvp(argv[2],argv + 2);
    if (error_temp(errno)) _exit(111);
    _exit(100);
  }
  if (wait_pid(&wstat,pid) == -1)
    strerr_die2x(111,FATAL,"wait failed");
  if (wait_crashed(wstat))
    strerr_die2x(111,FATAL,"child crashed");
  switch(wait_exitcode(wstat)) {
    case 0: break;
    case 111: strerr_die2x(111,FATAL,"temporary child error");
    default: _exit(0);
  }

  if (seek_begin(0) == -1)
    strerr_die2sys(111,FATAL,"unable to rewind: ");
  sig_pipeignore();
 
  sender = env_get("SENDER");
  if (!sender) strerr_die2x(100,FATAL,"SENDER not set");
  dtline = env_get("DTLINE");
  if (!dtline) strerr_die2x(100,FATAL,"DTLINE not set");
 
  if (qmail_open(&qqt) == -1)
    strerr_die2sys(111,FATAL,"unable to fork: ");
  qmail_puts(&qqt,dtline);
  if (substdio_copy(&ssout,&ssin) != 0)
    strerr_die2sys(111,FATAL,"unable to read message: ");
  substdio_flush(&ssout);
 
  num[fmt_ulong(num,qmail_qp(&qqt))] = 0;

  qmail_from(&qqt,sender);
  qmail_to(&qqt,argv[1]);
  qqx = qmail_close(&qqt);
  if (*qqx) strerr_die2x(*qqx == 'D' ? 100 : 111,FATAL,qqx + 1);
  strerr_die2x(99,"condredirect: qp ",num);
}
