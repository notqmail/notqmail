#include "strerr.h"
#include "error.h"
#include "exit.h"

extern void init_uidgid();
extern void hier();

#define FATAL "instchown: fatal: "

void h(home,uid,gid,mode)
char *home;
int uid;
int gid;
int mode;
{
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
  if (chown(fifo,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",fifo,": ");
  if (chmod(fifo,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",fifo,": ");
}

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
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chown(file,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",file,": ");
  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",file,": ");
}

void main()
{
  umask(077);
  init_uidgid();
  hier();
  _exit(0);
}
