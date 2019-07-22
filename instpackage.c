#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "substdio.h"
#include "strerr.h"
#include "env.h"
#include "error.h"
#include "fifo.h"
#include "hier.h"
#include "open.h"
#include "alloc.h"
#include "str.h"
#include "stralloc.h"

extern void hier();

#define FATAL "instpackage: fatal: "

int fdsourcedir = -1;

static void die_nomem()
{
  strerr_die2sys(111,FATAL,"out of memory");
}

static void ddhome(stralloc *dd, char *home)
{
  const char *denv = env_get("DESTDIR");
  if (denv)
    if (!stralloc_copys(dd,denv)) die_nomem();

  if (!stralloc_catb(dd,home,str_len(home))) die_nomem();
  if (!stralloc_0(dd)) die_nomem();
}

static int mkdir_p(char *home, int mode)
{
  stralloc parent = { 0 };
  unsigned int sl;
  int r = mkdir(home,mode);
  if (!r || errno != error_noent)
    return r;

  /* try parent first */
  sl = str_rchr(home, '/');
  if (!stralloc_copyb(&parent,home,sl)) die_nomem();
  if (!stralloc_0(&parent)) die_nomem();
  r = mkdir_p(parent.s,mode);
  alloc_free(parent.s);
  if (r && errno != error_exist)
    return r;

  return mkdir(home,mode);
}

void h(char *home, uid_t uid, gid_t gid, int mode)
{
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if (mkdir_p(home,mode) == -1)
    if (errno != error_exist)
      strerr_die4sys(111,FATAL,"unable to mkdir ",home,": ");
  if (chmod(home,mode) == -1)
    strerr_die4sys(111,FATAL,"unable to chmod ",home,": ");
  alloc_free(dh.s);
}

void d(char *home, char *subdir, uid_t uid, gid_t gid, int mode)
{
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (mkdir(subdir,0700) == -1)
    if (errno != error_exist)
      strerr_die6sys(111,FATAL,"unable to mkdir ",home,"/",subdir,": ");
  if (chmod(subdir,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",subdir,": ");
  alloc_free(dh.s);
}

void p(char *home, char *fifo, uid_t uid, gid_t gid, int mode)
{
  stralloc dh = { 0 };
  ddhome(&dh, home);
  home=dh.s;
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (fifo_make(fifo,0700) == -1)
    if (errno != error_exist)
      strerr_die6sys(111,FATAL,"unable to mkfifo ",home,"/",fifo,": ");
  if (chmod(fifo,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",fifo,": ");
  alloc_free(dh.s);
}

char inbuf[SUBSTDIO_INSIZE];
char outbuf[SUBSTDIO_OUTSIZE];
substdio ssin;
substdio ssout;

void c(char *home, char *subdir, char *file, uid_t uid, gid_t gid, int mode)
{
  int fdin;
  int fdout;
  stralloc dh = { 0 };

  if (fchdir(fdsourcedir) == -1)
    strerr_die2sys(111,FATAL,"unable to switch back to source directory: ");

  fdin = open_read(file);
  if (fdin == -1) {
    /* silently ignore missing catman pages */
    if (errno == error_noent && strncmp(subdir, "man/cat", 7) == 0)
      return;
    strerr_die4sys(111,FATAL,"unable to read ",file,": ");
  }

  /* if the user decided to build only dummy catman pages then don't install */
  if (strncmp(subdir, "man/cat", 7) == 0) {
    struct stat st;
    if (fstat(fdin, &st) != 0)
      strerr_die4sys(111,FATAL,"unable to stat ",file,": ");
    if (st.st_size == 0) {
      close(fdin);
      return;
    }
  }

  ddhome(&dh, home);
  home=dh.s;

  substdio_fdbuf(&ssin,read,fdin,inbuf,sizeof inbuf);

  if (chdir(home) == -1)
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

  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod .../",subdir,"/",file,": ");
  alloc_free(dh.s);
}

void z(char *home, char *file, int len, uid_t uid, gid_t gid, int mode)
{
  int fdout;
  stralloc dh = { 0 };

  ddhome(&dh, home);
  home=dh.s;
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

  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",home,"/",file,": ");
  alloc_free(dh.s);
}

/* these are ignored, but hier() passes them to h() and friends */
uid_t auto_uida = -1;
uid_t auto_uido = -1;
uid_t auto_uidq = -1;
uid_t auto_uidr = -1;
uid_t auto_uids = -1;

gid_t auto_gidq = -1;

void main()
{
  fdsourcedir = open_read(".");
  if (fdsourcedir == -1)
    strerr_die2sys(111,FATAL,"unable to open current directory: ");

  umask(077);
  hier();
  _exit(0);
}
