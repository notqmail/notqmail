#include "sig.h"
#include "readwrite.h"
#include "exit.h"
#include "env.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"

#define FATAL "forward: fatal: "

void die_nomem() { strerr_die2x(111,FATAL,"out of memory"); }

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
  char *qqx;
 
  sig_pipeignore();
 
  sender = env_get("NEWSENDER");
  if (!sender)
    strerr_die2x(100,FATAL,"NEWSENDER not set");
  dtline = env_get("DTLINE");
  if (!dtline)
    strerr_die2x(100,FATAL,"DTLINE not set");
 
  if (qmail_open(&qqt) == -1)
    strerr_die2sys(111,FATAL,"unable to fork: ");
  qmail_puts(&qqt,dtline);
  if (substdio_copy(&ssout,&ssin) != 0)
    strerr_die2sys(111,FATAL,"unable to read message: ");
  substdio_flush(&ssout);

  num[fmt_ulong(num,qmail_qp(&qqt))] = 0;
 
  qmail_from(&qqt,sender);
  while (*++argv) qmail_to(&qqt,*argv);
  qqx = qmail_close(&qqt);
  if (*qqx) strerr_die2x(*qqx == 'D' ? 100 : 111,FATAL,qqx + 1);
  strerr_die2x(0,"forward: qp ",num);
}
