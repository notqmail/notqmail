#include "substdio.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "readwrite.h"
#include "exit.h"
#include "str.h"
#include "env.h"

extern void hier();

#define FATAL "install-destdir: fatal: "

void h(home,uid,gid,mode)
char *home;
int uid;
int gid;
int mode;
{
  if (mkdir(home,0700) == -1)
    if (errno != error_exist)
      strerr_die4sys(111,FATAL,"unable to mkdir ",home,": ");
  if (chown(home,uid,gid) == -1)
    strerr_die4sys(111,FATAL,"unable to chown ",home,": ");
  if (chmod(home,mode) == -1)
    strerr_die4sys(111,FATAL,"unable to chmod ",home,": ");
}

void d(home,subdir,uid,gid,mode)
char *home;
char *subdir;
int uid;
int gid;
int mode;
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (mkdir(subdir,0700) == -1)
    if (errno != error_exist)
      strerr_die6sys(111,FATAL,"unable to mkdir ",home,"/",subdir,": ");
  if (chown(subdir,uid,gid) == -1)
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
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (fifo_make(fifo,0700) == -1)
    if (errno != error_exist)
      strerr_die6sys(111,FATAL,"unable to mkfifo ",home,"/",fifo,": ");
  if (chown(fifo,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",fifo,": ");
  if (chmod(fifo,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",fifo,": ");
}

char outbuf[SUBSTDIO_OUTSIZE];
substdio ssout;

void c(home,subdir,file,uid,gid,mode)
char *home;
char *subdir;
char *file;
int uid;
int gid;
int mode;
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chdir(subdir) == -1)
    strerr_die6sys(111,FATAL,"unable to switch to ",home,"/",subdir,": ");
  if (chown(file,uid,gid) == -1)
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

  if (chdir(home) == -1)
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

  if (chown(file,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",file,": ");
  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",file,": ");
}

void main()
{
  umask(077);
  hier();
  _exit(0);
}
