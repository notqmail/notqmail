#include "substdio.h"
#include "subfd.h"
#include "fmt.h"
#include "select.h"
#include "exit.h"
#include "auto_spawn.h"

char num[FMT_ULONG];
fd_set fds;

void main()
{
  unsigned long hiddenlimit;
  unsigned long maxnumd;
 
  hiddenlimit = sizeof(fds) * 8;
  maxnumd = (hiddenlimit - 5) / 2;
 
  if (auto_spawn < 1) {
    substdio_puts(subfderr,"Oops. You have set conf-spawn lower than 1.\n");
    substdio_flush(subfderr);
    _exit(1);
  }

  if (auto_spawn > 255) {
    substdio_puts(subfderr,"Oops. You have set conf-spawn higher than 255.\n");
    substdio_flush(subfderr);
    _exit(1);
  }

  if (auto_spawn > maxnumd) {
    substdio_puts(subfderr,"Oops. Your system's FD_SET() has a hidden limit of ");
    substdio_put(subfderr,num,fmt_ulong(num,hiddenlimit));
    substdio_puts(subfderr," descriptors.\n\
This means that the qmail daemons could crash if you set the run-time\n\
concurrency higher than ");
    substdio_put(subfderr,num,fmt_ulong(num,maxnumd));
    substdio_puts(subfderr,". So I'm going to insist that the concurrency\n\
limit in conf-spawn be at most ");
    substdio_put(subfderr,num,fmt_ulong(num,maxnumd));
    substdio_puts(subfderr,". Right now it's ");
    substdio_put(subfderr,num,fmt_ulong(num,(unsigned long) auto_spawn));
    substdio_puts(subfderr,".\n");
    substdio_flush(subfderr);
    _exit(1);
  }
  _exit(0);
}
