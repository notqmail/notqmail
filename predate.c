#include <sys/types.h>
#include <time.h>
#include "datetime.h"
#include "fork.h"
#include "wait.h"
#include "fd.h"
#include "fmt.h"
#include "strerr.h"
#include "substdio.h"
#include "subfd.h"
#include "readwrite.h"
#include "exit.h"

#define FATAL "predate: fatal: "

static char *montab[12] = {
"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

char num[FMT_ULONG];
char outbuf[1024];

void main(argc,argv)
int argc;
char **argv;
{
  time_t now;
  struct tm *tm;
  struct datetime dt;
  datetime_sec utc;
  datetime_sec local;
  int minutes;
  int pi[2];
  substdio ss;
  int wstat;
  int pid;

  sig_pipeignore();

  if (!argv[1])
    strerr_die1x(100,"predate: usage: predate child");

  if (pipe(pi) == -1)
    strerr_die2sys(111,FATAL,"unable to create pipe: ");

  switch(pid = fork()) {
    case -1:
      strerr_die2sys(111,FATAL,"unable to fork: ");
    case 0:
      close(pi[1]);
      if (fd_move(0,pi[0]) == -1)
	strerr_die2sys(111,FATAL,"unable to set up fds: ");
      sig_pipedefault();
      execvp(argv[1],argv + 1);
      strerr_die4sys(111,FATAL,"unable to run ",argv[1],": ");
  }
  close(pi[0]);
  substdio_fdbuf(&ss,write,pi[1],outbuf,sizeof(outbuf));

  time(&now);

  tm = gmtime(&now);
  dt.year = tm->tm_year;
  dt.mon = tm->tm_mon;
  dt.mday = tm->tm_mday;
  dt.hour = tm->tm_hour;
  dt.min = tm->tm_min;
  dt.sec = tm->tm_sec;
  utc = datetime_untai(&dt); /* utc == now, if gmtime ignores leap seconds */

  tm = localtime(&now);
  dt.year = tm->tm_year;
  dt.mon = tm->tm_mon;
  dt.mday = tm->tm_mday;
  dt.hour = tm->tm_hour;
  dt.min = tm->tm_min;
  dt.sec = tm->tm_sec;
  local = datetime_untai(&dt);

  substdio_puts(&ss,"Date: ");
  substdio_put(&ss,num,fmt_uint(num,dt.mday));
  substdio_puts(&ss," ");
  substdio_puts(&ss,montab[dt.mon]);
  substdio_puts(&ss," ");
  substdio_put(&ss,num,fmt_uint(num,dt.year + 1900));
  substdio_puts(&ss," ");
  substdio_put(&ss,num,fmt_uint0(num,dt.hour,2));
  substdio_puts(&ss,":");
  substdio_put(&ss,num,fmt_uint0(num,dt.min,2));
  substdio_puts(&ss,":");
  substdio_put(&ss,num,fmt_uint0(num,dt.sec,2));

  if (local < utc) {
    minutes = (utc - local + 30) / 60;
    substdio_puts(&ss," -");
    substdio_put(&ss,num,fmt_uint0(num,minutes / 60,2));
    substdio_put(&ss,num,fmt_uint0(num,minutes % 60,2));
  }
  else {
    minutes = (local - utc + 30) / 60;
    substdio_puts(&ss," +");
    substdio_put(&ss,num,fmt_uint0(num,minutes / 60,2));
    substdio_put(&ss,num,fmt_uint0(num,minutes % 60,2));
  }

  substdio_puts(&ss,"\n");
  substdio_copy(&ss,subfdin);
  substdio_flush(&ss);
  close(pi[1]);

  if (wait_pid(&wstat,pid) == -1)
    strerr_die2sys(111,FATAL,"wait failed: ");
  if (wait_crashed(wstat))
    strerr_die2x(111,FATAL,"child crashed");
  _exit(wait_exitcode(wstat));
}
