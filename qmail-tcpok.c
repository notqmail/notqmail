#include "strerr.h"
#include "substdio.h"
#include "lock.h"
#include "open.h"
#include "readwrite.h"
#include "auto_qmail.h"
#include "exit.h"

#define FATAL "qmail-tcpok: fatal: "

char buf[1024]; /* XXX: must match size in tcpto_clean.c, tcpto.c */
substdio ss;

void main()
{
  int fd;
  int i;

  if (chdir(auto_qmail) == -1)
    strerr_die4sys(111,FATAL,"unable to chdir to ",auto_qmail,": ");
  if (chdir("queue/lock") == -1)
    strerr_die4sys(111,FATAL,"unable to chdir to ",auto_qmail,"/queue/lock: ");

  fd = open_write("tcpto");
  if (fd == -1)
    strerr_die4sys(111,FATAL,"unable to write ",auto_qmail,"/queue/lock/tcpto: ");
  if (lock_ex(fd) == -1)
    strerr_die4sys(111,FATAL,"unable to lock ",auto_qmail,"/queue/lock/tcpto: ");

  substdio_fdbuf(&ss,write,fd,buf,sizeof buf);
  for (i = 0;i < sizeof buf;++i) substdio_put(&ss,"",1);
  if (substdio_flush(&ss) == -1)
    strerr_die4sys(111,FATAL,"unable to clear ",auto_qmail,"/queue/lock/tcpto: ");
  _exit(0);
}
