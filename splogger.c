#include <sys/types.h>
#include <sys/time.h>
#include <syslog.h>
#include "error.h"
#include "substdio.h"
#include "subfd.h"
#include "exit.h"
#include "str.h"
#include "scan.h"
#include "fmt.h"

char buf[800]; /* syslog truncates long lines (or crashes); GPACIC */
int bufpos = 0; /* 0 <= bufpos < sizeof(buf) */
int flagcont = 0;
int priority; /* defined if flagcont */
char stamp[FMT_ULONG + FMT_ULONG + 3]; /* defined if flagcont */

void stamp_make()
{
  struct timeval tv;
  char *s;
  gettimeofday(&tv,(struct timezone *) 0);
  s = stamp;
  s += fmt_ulong(s,(unsigned long) tv.tv_sec);
  *s++ = '.';
  s += fmt_uint0(s,(unsigned int) tv.tv_usec,6);
  *s = 0;
}

void flush()
{
  if (bufpos) {
    buf[bufpos] = 0;
    if (flagcont)
      syslog(priority,"%s+%s",stamp,buf); /* logger folds invisibly; GPACIC */
    else {
      stamp_make();
      priority = LOG_INFO;
      if (str_start(buf,"warning:")) priority = LOG_WARNING;
      if (str_start(buf,"alert:")) priority = LOG_ALERT;
      syslog(priority,"%s %s",stamp,buf);
      flagcont = 1;
    }
  }
  bufpos = 0;
}

void main(argc,argv)
int argc;
char **argv;
{
  char ch;

  if (argv[1])
    if (argv[2]) {
      unsigned long facility;
      scan_ulong(argv[2],&facility);
      openlog(argv[1],0,facility << 3);
    }
    else
      openlog(argv[1],0,LOG_MAIL);
  else
    openlog("splogger",0,LOG_MAIL);

  for (;;) {
    if (substdio_get(subfdin,&ch,1) < 1) _exit(0);
    if (ch == '\n') { flush(); flagcont = 0; continue; }
    if (bufpos == sizeof(buf) - 1) flush();
    if ((ch < 32) || (ch > 126)) ch = '?'; /* logger truncates at 0; GPACIC */
    buf[bufpos++] = ch;
  }
}
