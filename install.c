#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "substdio.h"
#include "strerr.h"
#include "fifo.h"
#include "error.h"
#include "open.h"
#include "exit.h"
#include "stralloc.h"
#include "sgetopt.h"

extern void hier();

#define FATAL "install: fatal: "

int fdsourcedir = -1;
uid_t my_uid;
gid_t my_gid;
char *destdir = 0;
stralloc tmpdir = { 0 };
stralloc dirbuf = { 0 };

int
r_mkdir(home, mode)
	char           *home;
	int             mode;
{
	char           *ptr;
	int             i;

	if (!stralloc_copys(&dirbuf, home))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&dirbuf))
		strerr_die2sys(111, FATAL, "out of memory: ");
	for (ptr = dirbuf.s + 1;*ptr;ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			if (access(dirbuf.s, F_OK) && (i = mkdir(dirbuf.s, mode)) == -1)
				return (i);
			*ptr = '/';
		}
	}
	return (mkdir(dirbuf.s, mode));
}

void h(home,uid,gid,mode)
char *home;
int uid;
int gid;
int mode;
{
  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111, FATAL, "out of memory: ");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111, FATAL, "out of memory: ");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (r_mkdir(tmpdir.s,0700) == -1)
    if (errno != error_exist)
      strerr_die4sys(111,FATAL,"unable to mkdir ",home,": ");
  if (!my_uid && chown(tmpdir.s,uid,gid) == -1)
    strerr_die4sys(111,FATAL,"unable to chown ",home,": ");
  if (chmod(tmpdir.s,mode) == -1)
    strerr_die4sys(111,FATAL,"unable to chmod ",home,": ");
}

void d(home,subdir,uid,gid,mode)
char *home;
char *subdir;
int uid;
int gid;
int mode;
{
  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111, FATAL, "out of memory: ");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111, FATAL, "out of memory: ");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (chdir(tmpdir.s) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (mkdir(subdir,0700) == -1)
    if (errno != error_exist)
      strerr_die6sys(111,FATAL,"unable to mkdir ",home,"/",subdir,": ");
  if (!my_uid && chown(subdir,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",subdir,": ");
  if (chmod(subdir,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",subdir,": ");
}

void p(home,fifo,uid,gid,mode)
char *home;
char *fifo;
int uid;
int gid;
int mode;
{
  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111, FATAL, "out of memory: ");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111, FATAL, "out of memory: ");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (chdir(tmpdir.s) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (fifo_make(fifo,0700) == -1)
    if (errno != error_exist)
      strerr_die6sys(111,FATAL,"unable to mkfifo ",home,"/",fifo,": ");
  if (!my_uid && chown(fifo,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",fifo,": ");
  if (chmod(fifo,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",fifo,": ");
}

char inbuf[SUBSTDIO_INSIZE];
char outbuf[SUBSTDIO_OUTSIZE];
substdio ssin;
substdio ssout;

void c(home,subdir,file,uid,gid,mode)
char *home;
char *subdir;
char *file;
int uid;
int gid;
int mode;
{
  int fdin;
  int fdout;

  if (fchdir(fdsourcedir) == -1)
    strerr_die2sys(111,FATAL,"unable to switch back to source directory: ");

  fdin = open_read(file);
  if (fdin == -1)
    strerr_die4sys(111,FATAL,"unable to read ",file,": ");
  substdio_fdbuf(&ssin,read,fdin,inbuf,sizeof inbuf);

  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111, FATAL, "out of memory: ");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111, FATAL, "out of memory: ");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111, FATAL, "out of memory: ");

  if (chdir(tmpdir.s) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chdir(subdir) == -1)
    strerr_die6sys(111,FATAL,"unable to switch to ",home,"/",subdir,": ");

  fdout = open_trunc(file);
  if (fdout == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  substdio_fdbuf(&ssout,write,fdout,outbuf,sizeof outbuf);

  switch(substdio_copy(&ssout,&ssin)) {
    case -2:
      strerr_die4sys(111,FATAL,"unable to read ",file,": ");
    case -3:
      strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  }

  close(fdin);
  if (substdio_flush(&ssout) == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  if (fsync(fdout) == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  if (close(fdout) == -1) /* NFS silliness */
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");

  if (!my_uid && chown(file,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown .../",subdir,"/",file,": ");
  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod .../",subdir,"/",file,": ");
}

void z(home,file,len,uid,gid,mode)
char *home;
char *file;
int len;
int uid;
int gid;
int mode;
{
  int fdout;

  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111, FATAL, "out of memory: ");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111, FATAL, "out of memory: ");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111, FATAL, "out of memory: ");
  if (chdir(tmpdir.s) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");

  fdout = open_trunc(file);
  if (fdout == -1)
    strerr_die6sys(111,FATAL,"unable to write ",home,"/",file,": ");
  substdio_fdbuf(&ssout,write,fdout,outbuf,sizeof outbuf);

  while (len-- > 0)
    if (substdio_put(&ssout,"",1) == -1)
      strerr_die6sys(111,FATAL,"unable to write ",home,"/",file,": ");

  if (substdio_flush(&ssout) == -1)
    strerr_die6sys(111,FATAL,"unable to write ",home,"/",file,": ");
  if (fsync(fdout) == -1)
    strerr_die6sys(111,FATAL,"unable to write ",home,"/",file,": ");
  if (close(fdout) == -1) /* NFS silliness */
    strerr_die6sys(111,FATAL,"unable to write ",home,"/",file,": ");

  if (!my_uid && chown(file,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",file,": ");
  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",file,": ");
}

char           *usage = "usage: setup -d destdir";

void main(int argc, char **argv)
{
  int  opt;

  my_uid = getuid();
  my_gid = getgid();
  fdsourcedir = open_read(".");
  if (fdsourcedir == -1)
    strerr_die2sys(111,FATAL,"unable to open current directory: ");
  while ((opt = getopt(argc, argv, "ld:s:L:")) != opteof) {
    switch (opt)
    {
      case 'd':
        destdir = optarg;
        break;
      default:
        strerr_die1x(100, usage);
    }
  }
  if (destdir && !*destdir)
    destdir = 0;

  umask(077);
  hier();
  _exit(0);
}
