#include "substdio.h"
#include "stralloc.h"
#include "getln.h"
#include "readwrite.h"
#include "exit.h"
#include "open.h"
#include "error.h"
#include "strerr.h"
#include "byte.h"
#include "fifo.h"

stralloc target = {0};
char *to;

#define FATAL "install: fatal: "
void nomem() { strerr_die2x(111,FATAL,"out of memory"); }

char inbuf[SUBSTDIO_INSIZE];
char outbuf[SUBSTDIO_OUTSIZE];
substdio ssin;
substdio ssout;

void doit(line)
stralloc *line;
{
  char *x;
  unsigned int xlen;
  unsigned int i;
  char *type;
  char *uidstr;
  char *gidstr;
  char *modestr;
  char *mid;
  char *name;
  unsigned long uid;
  unsigned long gid;
  unsigned long mode;
  int fdin;
  int fdout;
  unsigned long zlen;

  x = line->s; xlen = line->len;

  type = x;
  i = byte_chr(x,xlen,':'); if (i == xlen) return;
  x[i++] = 0; x += i; xlen -= i;

  uidstr = x;
  i = byte_chr(x,xlen,':'); if (i == xlen) return;
  x[i++] = 0; x += i; xlen -= i;

  gidstr = x;
  i = byte_chr(x,xlen,':'); if (i == xlen) return;
  x[i++] = 0; x += i; xlen -= i;

  modestr = x;
  i = byte_chr(x,xlen,':'); if (i == xlen) return;
  x[i++] = 0; x += i; xlen -= i;

  mid = x;
  i = byte_chr(x,xlen,':'); if (i == xlen) return;
  x[i++] = 0; x += i; xlen -= i;

  name = x;
  i = byte_chr(x,xlen,':'); if (i == xlen) return;
  x[i++] = 0; x += i; xlen -= i;

  if (!stralloc_copys(&target,to)) nomem();
  if (!stralloc_cats(&target,mid)) nomem();
  if (!stralloc_cats(&target,name)) nomem();
  if (!stralloc_0(&target)) nomem();

  uid = -1; if (*uidstr) scan_ulong(uidstr,&uid);
  gid = -1; if (*gidstr) scan_ulong(gidstr,&gid);
  scan_8long(modestr,&mode);

  switch(*type) {
    case 'z':
      scan_ulong(type + 1,&zlen);

      fdout = open_trunc(target.s);
      if (fdout == -1)
	strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");
      substdio_fdbuf(&ssout,write,fdout,outbuf,sizeof(outbuf));

      while (zlen--)
	if (substdio_put(&ssout,"",1) == -1)
	  strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");

      if (substdio_flush(&ssout) == -1)
	strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");
      if (fsync(fdout) == -1)
	strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");
      close(fdout);
      break;

    case 'p':
      if (fifo_make(target.s,0700) == -1)
        if (errno != error_exist)
	  strerr_die4sys(111,FATAL,"unable to mkfifo ",target.s,": ");
      break;

    case 'd':
      if (mkdir(target.s,0700) == -1)
        if (errno != error_exist)
	  strerr_die4sys(111,FATAL,"unable to mkdir ",target.s,": ");
      break;

    case 'c':
      fdin = open_read(name);
      if (fdin == -1)
	strerr_die4sys(111,FATAL,"unable to read ",name,": ");
      substdio_fdbuf(&ssin,read,fdin,inbuf,sizeof(inbuf));

      fdout = open_trunc(target.s);
      if (fdout == -1)
	strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");
      substdio_fdbuf(&ssout,write,fdout,outbuf,sizeof(outbuf));

      switch(substdio_copy(&ssout,&ssin)) {
	case -2:
	  strerr_die4sys(111,FATAL,"unable to read ",name,": ");
	case -3:
	  strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");
      }

      close(fdin);
      if (substdio_flush(&ssout) == -1)
	strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");
      if (fsync(fdout) == -1)
	strerr_die4sys(111,FATAL,"unable to write ",target.s,": ");
      close(fdout);
      break;

    default:
      return;
  }

  if (chown(target.s,uid,gid) == -1)
    strerr_die4sys(111,FATAL,"unable to chown ",target.s,": ");
  if (chmod(target.s,mode) == -1)
    strerr_die4sys(111,FATAL,"unable to chmod ",target.s,": ");
}

char buf[256];
substdio in = SUBSTDIO_FDBUF(read,0,buf,sizeof(buf));
stralloc line = {0};

void main(argc,argv)
int argc;
char **argv;
{
  int match;

  umask(077);

  to = argv[1];
  if (!to) strerr_die2x(100,FATAL,"install: usage: install dir");

  for (;;) {
    if (getln(&in,&line,&match,'\n') == -1)
      strerr_die2sys(111,FATAL,"unable to read input: ");
    doit(&line);
    if (!match)
      _exit(0);
  }
}
