#include <sys/types.h>
#include <sys/stat.h>
#include "substdio.h"
#include "stralloc.h"
#include "getln.h"
#include "readwrite.h"
#include "exit.h"
#include "error.h"
#include "strerr.h"
#include "byte.h"

stralloc target = {0};
char *to;

#define WARNING "instcheck: warning: "
#define FATAL "instcheck: fatal: "
void nomem() { strerr_die2x(111,FATAL,"out of memory"); }

void doit(line)
stralloc *line;
{
  struct stat st;
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
  int ftype;

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
    case 'd': ftype = S_IFDIR; break;
    case 'c': ftype = S_IFREG; break;
    case 'z': ftype = S_IFREG; break;
    case 'p': ftype = S_IFIFO; break;
    default: return;
  }

  if (stat(target.s,&st) == -1) {
    if (errno == error_noent)
      strerr_warn3(WARNING,target.s," does not exist",0);
    else
      strerr_warn4(WARNING,"unable to stat ",target.s,": ",&strerr_sys);
    return;
  }

  if ((uid != -1) && (st.st_uid != uid))
    strerr_warn3(WARNING,target.s," has wrong owner",0);
  if ((gid != -1) && (st.st_gid != gid))
    strerr_warn3(WARNING,target.s," has wrong group",0);
  if ((st.st_mode & 07777) != mode)
    strerr_warn3(WARNING,target.s," has wrong permissions",0);
  if ((st.st_mode & S_IFMT) != ftype)
    strerr_warn3(WARNING,target.s," has wrong type",0);
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
  if (!to) strerr_die2x(100,FATAL,"instcheck: usage: instcheck dir");

  for (;;) {
    if (getln(&in,&line,&match,'\n') == -1)
      strerr_die2sys(111,FATAL,"unable to read input: ");
    doit(&line);
    if (!match)
      _exit(0);
  }
}
