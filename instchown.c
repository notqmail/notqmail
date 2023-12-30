#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "strerr.h"
#include "error.h"
#include "hier.h"

extern void init_uidgid();
extern void hier();

#define FATAL "instchown: fatal: "

void h(char *home, uid_t uid, gid_t gid, int mode)
{
  if (chown(home,uid,gid) == -1)
    strerr_die4sys(111,FATAL,"unable to chown ",home,": ");
  if (chmod(home,mode) == -1)
    strerr_die4sys(111,FATAL,"unable to chmod ",home,": ");
}

void d(char *home, char *subdir, uid_t uid, gid_t gid, int mode)
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chown(subdir,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",subdir,": ");
  if (chmod(subdir,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",subdir,": ");
}

void p(char *home, char *fifo, uid_t uid, gid_t gid, int mode)
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chown(fifo,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",home,"/",fifo,": ");
  if (chmod(fifo,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",fifo,": ");
}

void c(char *home, char *subdir, char *file, uid_t uid, gid_t gid, int mode)
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chdir(subdir) == -1) {
    /* assume cat man pages are simply not installed */
    if (errno == error_noent && strncmp(subdir, "man/cat", 7) == 0)
      return;
    strerr_die6sys(111,FATAL,"unable to switch to ",home,"/",subdir,": ");
  }
  if (chown(file,uid,gid) == -1) {
    /* assume cat man pages are simply not installed */
    if (errno == error_noent && strncmp(subdir, "man/cat", 7) == 0)
      return;
    strerr_die6sys(111,FATAL,"unable to chown .../",subdir,"/",file,": ");
  }
  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod .../",subdir,"/",file,": ");
}

void z(char *home, char *file, int len, uid_t uid, gid_t gid, int mode)
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
