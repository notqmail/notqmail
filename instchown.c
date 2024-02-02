#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "env.h"
#include "open.h"
#include "stralloc.h"
#include "strerr.h"
#include "error.h"
#include "hier.h"

extern void init_uidgid();

#define FATAL "instchown: fatal: "

static void die_nomem()
{
  strerr_die2sys(111,FATAL,"out of memory");
}

static void ddhome(stralloc *dd, const char *home)
{
  const char *denv = env_get("DESTDIR");
  if (denv)
    if (!stralloc_copys(dd,denv)) die_nomem();

  if (!stralloc_catb(dd,home,strlen(home))) die_nomem();
  if (!stralloc_0(dd)) die_nomem();
}

void h(char *home, uid_t uid, gid_t gid, int mode)
{
  int fd;
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if ((fd = open_read(home)) >= 0) {
    if (fchown(fd,uid,gid) == -1)
      strerr_die4sys(111,FATAL,"unable to chown ",home,": ");
    if (fchmod(fd,mode) == -1)
      strerr_die4sys(111,FATAL,"unable to chmod ",home,": ");
    close(fd);
  } else {
    strerr_die4sys(111,FATAL,"unable to open ",home,": ");
  }
  free(dh.s);
}

void d(char *home, char *subdir, uid_t uid, gid_t gid, int mode)
{
  int fd;
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if ((fd = open_read(subdir)) >= 0) {
    if (fchown(fd,uid,gid) == -1)
      strerr_die6sys(111,FATAL,"unable to chown ",home,"/",subdir,": ");
    if (fchmod(fd,mode) == -1)
      strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",subdir,": ");
    close(fd);
  } else {
    strerr_die6sys(111,FATAL,"unable to open ",home,"/",subdir,": ");
  }
  free(dh.s);
}

void p(char *home, char *fifo, uid_t uid, gid_t gid, int mode)
{
  int fd;
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if ((fd = open_read(fifo)) >= 0) {
    if (fchown(fd,uid,gid) == -1)
      strerr_die6sys(111,FATAL,"unable to chown ",home,"/",fifo,": ");
    if (fchmod(fd,mode) == -1)
      strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",fifo,": ");
    close(fd);
  } else {
    strerr_die6sys(111,FATAL,"unable to open ",home,"/",fifo,": ");
  }
  free(dh.s);
}

void c(char *home, char *subdir, char *file, uid_t uid, gid_t gid, int mode)
{
  int fd;
  int iscatdir = (0 == strncmp(subdir, "man/cat", 7));
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chdir(subdir) == -1) {
    /* assume cat man pages are simply not installed */
    if (errno == error_noent && iscatdir)
      return;
    strerr_die6sys(111,FATAL,"unable to switch to ",home,"/",subdir,": ");
  }
  if ((fd = open_read(file)) >= 0) {
    if (fchown(fd,uid,gid) == -1) {
      /* assume cat man pages are simply not installed */
      if (errno == error_noent && iscatdir)
        return;
      strerr_die6sys(111,FATAL,"unable to chown .../",subdir,"/",file,": ");
    }
    if (fchmod(fd,mode) == -1)
      strerr_die6sys(111,FATAL,"unable to chmod .../",subdir,"/",file,": ");
    close(fd);
  } else {
    /* assume cat man pages are simply not installed */
    if (!iscatdir)
      strerr_die6sys(111,FATAL,"unable to open .../",subdir,"/",file,": ");
  }
  free(dh.s);
}

void z(char *home, char *file, int len, uid_t uid, gid_t gid, int mode)
{
  int fd;
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if ((fd = open_read(file)) >= 0) {
    if (fchown(fd,uid,gid) == -1)
      strerr_die6sys(111,FATAL,"unable to chown ",home,"/",file,": ");
    if (fchmod(fd,mode) == -1)
      strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",file,": ");
    close(fd);
  } else {
    strerr_die6sys(111,FATAL,"unable to open ",home,"/",file,": ");
  }
  free(dh.s);
}

int main(int argc, char **argv)
{
  umask(077);
  init_uidgid();
  if (argc == 2 && strcmp(argv[1],"queue-only") == 0)
    hier_queue();
  else
    hier();
  return 0;
}
