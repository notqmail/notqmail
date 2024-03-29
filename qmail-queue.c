#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "readwrite.h"
#include "sig.h"
#include "exit.h"
#include "open.h"
#include "seek.h"
#include "fmt.h"
#include "alloc.h"
#include "substdio.h"
#include "datetime.h"
#include "now.h"
#include "triggerpull.h"
#include "extra.h"
#include "noreturn.h"
#include "uidgid.h"
#include "auto_qmail.h"
#include "auto_uids.h"
#include "auto_users.h"
#include "date822fmt.h"
#include "fmtqfn.h"

#define DEATH 86400 /* 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */
#define ADDR 1003

char inbuf[2048];
struct substdio ssin;
char outbuf[256];
struct substdio ssout;

datetime_sec starttime;
struct datetime dt;
unsigned long mypid;
uid_t uid;
char *pidfn;
struct stat pidst;
unsigned long messnum;
char *messfn;
char *todofn;
char *intdfn;
int messfd;
int intdfd;
int flagmademess = 0;
int flagmadeintd = 0;

uid_t auto_uida;
uid_t auto_uidd;
uid_t auto_uids;

void cleanup()
{
 if (flagmadeintd)
  {
   seek_trunc(intdfd,0);
   if (unlink(intdfn) == -1) return;
  }
 if (flagmademess)
  {
   seek_trunc(messfd,0);
   if (unlink(messfn) == -1) return;
  }
}

void _noreturn_ die(int e) { _exit(e); }
void _noreturn_ die_write() { cleanup(); die(53); }
void _noreturn_ die_read() { cleanup(); die(54); }
void _noreturn_ sigalrm() { /* thou shalt not clean up here */ die(52); }
void _noreturn_ sigbug() { die(81); }

unsigned int receivedlen;
char *received;
/* "Received: (qmail-queue invoked by alias); 26 Sep 1995 04:46:54 -0000\n" */

static unsigned int receivedfmt(s)
char *s;
{
 unsigned int i;
 unsigned int len;
 len = 0;
 i = fmt_str(s,"Received: (qmail "); len += i; if (s) s += i;
 i = fmt_ulong(s,mypid); len += i; if (s) s += i;
 i = fmt_str(s," invoked "); len += i; if (s) s += i;
 if (uid == auto_uida)
  { i = fmt_str(s,"by alias"); len += i; if (s) s += i; }
 else if (uid == auto_uidd)
  { i = fmt_str(s,"from network"); len += i; if (s) s += i; }
 else if (uid == auto_uids)
  { i = fmt_str(s,"for bounce"); len += i; if (s) s += i; }
 else
  {
   i = fmt_str(s,"by uid "); len += i; if (s) s += i;
   i = fmt_ulong(s,uid); len += i; if (s) s += i;
  }
 i = fmt_str(s,"); "); len += i; if (s) s += i;
 i = date822fmt(s,&dt); len += i; if (s) s += i;
 return len;
}

void received_setup()
{
 receivedlen = receivedfmt(NULL);
 received = alloc(receivedlen + 1);
 if (!received) die(51);
 receivedfmt(received);
}

unsigned int pidfmt(s,seq)
char *s;
unsigned long seq;
{
 unsigned int i;
 unsigned int len;

 len = 0;
 i = fmt_str(s,"pid/"); len += i; if (s) s += i;
 i = fmt_ulong(s,mypid); len += i; if (s) s += i;
 i = fmt_str(s,"."); len += i; if (s) s += i;
 i = fmt_ulong(s,starttime); len += i; if (s) s += i;
 i = fmt_str(s,"."); len += i; if (s) s += i;
 i = fmt_ulong(s,seq); len += i; if (s) s += i;
 ++len; if (s) *s++ = 0;

 return len;
}

char *fnnum(dirslash,flagsplit)
char *dirslash;
int flagsplit;
{
 char *s;

 s = alloc(fmtqfn(NULL,dirslash,messnum,flagsplit));
 if (!s) die(51);
 fmtqfn(s,dirslash,messnum,flagsplit);
 return s;
}

void pidopen()
{
 unsigned int len;
 unsigned long seq;

 seq = 1;
 len = pidfmt(NULL,seq);
 pidfn = alloc(len);
 if (!pidfn) die(51);

 for (seq = 1;seq < 10;++seq)
  {
   if (pidfmt(NULL,seq) > len) die(81); /* paranoia */
   pidfmt(pidfn,seq);
   messfd = open_excl(pidfn);
   if (messfd != -1) return;
  }

 die(63);
}

char tmp[FMT_ULONG];

int main(void)
{
 unsigned int len;
 char ch;

 sig_blocknone();
 umask(033);
 if (chdir(auto_qmail) == -1) die(61);
 if (chdir("queue") == -1) die(62);

 mypid = getpid();
 uid = getuid();
 starttime = now();
 datetime_tai(&dt,starttime);

 auto_uida = inituid(auto_usera);
 auto_uidd = inituid(auto_userd);
 auto_uids = inituid(auto_users);

 received_setup();

 sig_pipeignore();
 sig_miscignore();
 sig_alarmcatch(sigalrm);
 sig_bugcatch(sigbug);

 alarm(DEATH);

 pidopen();
 if (fstat(messfd,&pidst) == -1) die(63);

 messnum = pidst.st_ino;
 messfn = fnnum("mess/",1);
 todofn = fnnum("todo/",0);
 intdfn = fnnum("intd/",0);

 if (link(pidfn,messfn) == -1) die(64);
 if (unlink(pidfn) == -1) die(63);
 flagmademess = 1;

 substdio_fdbuf(&ssout,write,messfd,outbuf,sizeof(outbuf));
 substdio_fdbuf(&ssin,read,0,inbuf,sizeof(inbuf));

 if (substdio_bput(&ssout,received,receivedlen) == -1) die_write();

 switch(substdio_copy(&ssout,&ssin))
  {
   case -2: die_read();
   case -3: die_write();
  }

 if (substdio_flush(&ssout) == -1) die_write();
 if (fsync(messfd) == -1) die_write();

 intdfd = open_excl(intdfn);
 if (intdfd == -1) die(65);
 flagmadeintd = 1;

 substdio_fdbuf(&ssout,write,intdfd,outbuf,sizeof(outbuf));
 substdio_fdbuf(&ssin,read,1,inbuf,sizeof(inbuf));

 if (substdio_bput(&ssout,"u",1) == -1) die_write();
 if (substdio_bput(&ssout,tmp,fmt_ulong(tmp,uid)) == -1) die_write();
 if (substdio_bput(&ssout,"",1) == -1) die_write();

 if (substdio_bput(&ssout,"p",1) == -1) die_write();
 if (substdio_bput(&ssout,tmp,fmt_ulong(tmp,mypid)) == -1) die_write();
 if (substdio_bput(&ssout,"",1) == -1) die_write();

 if (substdio_get(&ssin,&ch,1) < 1) die_read();
 if (ch != 'F') die(91);
 if (substdio_bput(&ssout,&ch,1) == -1) die_write();
 for (len = 0;len < ADDR;++len)
  {
   if (substdio_get(&ssin,&ch,1) < 1) die_read();
   if (substdio_put(&ssout,&ch,1) == -1) die_write();
   if (!ch) break;
  }
 if (len >= ADDR) die(11);

 if (substdio_bput(&ssout,QUEUE_EXTRA,QUEUE_EXTRALEN) == -1) die_write();

 for (;;)
  {
   if (substdio_get(&ssin,&ch,1) < 1) die_read();
   if (!ch) break;
   if (ch != 'T') die(91);
   if (substdio_bput(&ssout,&ch,1) == -1) die_write();
   for (len = 0;len < ADDR;++len)
    {
     if (substdio_get(&ssin,&ch,1) < 1) die_read();
     if (substdio_bput(&ssout,&ch,1) == -1) die_write();
     if (!ch) break;
    }
   if (len >= ADDR) die(11);
  }

 if (substdio_flush(&ssout) == -1) die_write();
 if (fsync(intdfd) == -1) die_write();

 if (link(intdfn,todofn) == -1) die(66);

 triggerpull();
 return 0;
}
