#include <sys/types.h>
#include <sys/stat.h>
#include "prioq.h"
#include "env.h"
#include "stralloc.h"
#include "direntry.h"
#include "datetime.h"
#include "now.h"
#include "str.h"
#include "maildir.h"

struct strerr maildir_chdir_err;
struct strerr maildir_scan_err;

int maildir_chdir()
{
 char *maildir;
 maildir = env_get("MAILDIR");
 if (!maildir)
   STRERR(-1,maildir_chdir_err,"MAILDIR not set")
 if (chdir(maildir) == -1)
   STRERR_SYS3(-1,maildir_chdir_err,"unable to chdir to ",maildir,": ")
 return 0;
}

void maildir_clean(tmpname)
stralloc *tmpname;
{
 DIR *dir;
 direntry *d;
 datetime_sec time;
 struct stat st;

 time = now();

 dir = opendir("tmp");
 if (!dir) return;

 while (d = readdir(dir))
  {
   if (d->d_name[0] == '.') continue;
   if (!stralloc_copys(tmpname,"tmp/")) break;
   if (!stralloc_cats(tmpname,d->d_name)) break;
   if (!stralloc_0(tmpname)) break;
   if (stat(tmpname->s,&st) == 0)
     if (time > st.st_atime + 129600)
       unlink(tmpname->s);
  }
 closedir(dir);
}

static int append(pq,filenames,subdir,time)
prioq *pq;
stralloc *filenames;
char *subdir;
datetime_sec time;
{
 DIR *dir;
 direntry *d;
 struct prioq_elt pe;
 unsigned int pos;
 struct stat st;

 dir = opendir(subdir);
 if (!dir)
   STRERR_SYS3(-1,maildir_scan_err,"unable to scan $MAILDIR/",subdir,": ")

 while (d = readdir(dir))
  {
   if (d->d_name[0] == '.') continue;
   pos = filenames->len;
   if (!stralloc_cats(filenames,subdir)) break;
   if (!stralloc_cats(filenames,"/")) break;
   if (!stralloc_cats(filenames,d->d_name)) break;
   if (!stralloc_0(filenames)) break;
   if (stat(filenames->s + pos,&st) == 0)
     if (st.st_mtime < time) /* don't want to mix up the order */
      {
       pe.dt = st.st_mtime;
       pe.id = pos;
       if (!prioq_insert(pq,&pe)) break;
      }
  }

 closedir(dir);
 if (d) STRERR_SYS3(-1,maildir_scan_err,"unable to read $MAILDIR/",subdir,": ")
 return 0;
}

int maildir_scan(pq,filenames,flagnew,flagcur)
prioq *pq;
stralloc *filenames;
int flagnew;
int flagcur;
{
 struct prioq_elt pe;
 datetime_sec time;
 int r;

 if (!stralloc_copys(filenames,"")) return 0;
 while (prioq_min(pq,&pe)) prioq_delmin(pq);

 time = now();

 if (flagnew) if (append(pq,filenames,"new",time) == -1) return -1;
 if (flagcur) if (append(pq,filenames,"cur",time) == -1) return -1;
 return 0;
}
