#include <sys/types.h>
#include <sys/stat.h>
#include "readwrite.h"
#include "sig.h"
#include "now.h"
#include "str.h"
#include "direntry.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "byte.h"
#include "scan.h"
#include "fmt.h"
#include "error.h"
#include "exit.h"
#include "fmtqfn.h"
#include "auto_qmail.h"

#define OSSIFIED 129600 /* see qmail-send.c */

stralloc line = {0};

void cleanuppid()
{
 DIR *dir;
 direntry *d;
 struct stat st;
 datetime_sec time;

 time = now();
 dir = opendir("pid");
 if (!dir) return;
 while (d = readdir(dir))
  {
   if (str_equal(d->d_name,".")) continue;
   if (str_equal(d->d_name,"..")) continue;
   if (!stralloc_copys(&line,"pid/")) continue;
   if (!stralloc_cats(&line,d->d_name)) continue;
   if (!stralloc_0(&line)) continue;
   if (stat(line.s,&st) == -1) continue;
   if (time < st.st_atime + OSSIFIED) continue;
   unlink(line.s);
  }
 closedir(dir);
}

char fnbuf[FMTQFN];

void respond(s) char *s; { if (substdio_putflush(subfdoutsmall,s,1) == -1) _exit(100); }

void main()
{
 int i;
 int match;
 int cleanuploop;
 unsigned long id;

 if (chdir(auto_qmail) == -1) _exit(111);
 if (chdir("queue") == -1) _exit(111);

 sig_pipeignore();

 if (!stralloc_ready(&line,200)) _exit(111);

 cleanuploop = 0;

 for (;;)
  {
   if (cleanuploop) --cleanuploop; else { cleanuppid(); cleanuploop = 30; }
   if (getln(subfdinsmall,&line,&match,'\0') == -1) break;
   if (!match) break;
   if (line.len < 7) { respond("x"); continue; }
   if (line.len > 100) { respond("x"); continue; }
   if (line.s[line.len - 1]) { respond("x"); continue; } /* impossible */
   for (i = 5;i < line.len - 1;++i)
     if ((unsigned char) (line.s[i] - '0') > 9)
      { respond("x"); continue; }
   if (!scan_ulong(line.s + 5,&id)) { respond("x"); continue; }
   if (byte_equal(line.s,5,"foop/"))
    {
#define U(prefix,flag) fmtqfn(fnbuf,prefix,id,flag); \
if (unlink(fnbuf) == -1) if (errno != error_noent) { respond("!"); continue; }
     U("intd/",0)
     U("mess/",1)
     respond("+");
    }
   else if (byte_equal(line.s,4,"todo/"))
    {
     U("intd/",0)
     U("todo/",0)
     respond("+");
    }
   else
     respond("x");
  }
 _exit(0);
}
