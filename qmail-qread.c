#include <sys/types.h>
#include <sys/stat.h>
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "fmt.h"
#include "str.h"
#include "getln.h"
#include "fmtqfn.h"
#include "readsubdir.h"
#include "auto_qmail.h"
#include "open.h"
#include "datetime.h"
#include "date822fmt.h"
#include "readwrite.h"
#include "error.h"
#include "exit.h"

readsubdir rs;

void die(n) int n; { substdio_flush(subfdout); _exit(n); }

void warn(s1,s2) char *s1; char *s2;
{
 char *x;
 x = error_str(errno);
 substdio_puts(subfdout,s1);
 substdio_puts(subfdout,s2);
 substdio_puts(subfdout,": ");
 substdio_puts(subfdout,x);
 substdio_puts(subfdout,"\n");
}

void die_nomem() { substdio_puts(subfdout,"fatal: out of memory\n"); die(111); }
void die_chdir() { warn("fatal: unable to chdir",""); die(111); }
void die_opendir(fn) char *fn; { warn("fatal: unable to opendir ",fn); die(111); }

void err(id) unsigned long id;
{
 char foo[FMT_ULONG];
 foo[fmt_ulong(foo,id)] = 0;
 warn("warning: trouble with #",foo);
}

char fnmess[FMTQFN];
char fninfo[FMTQFN];
char fnlocal[FMTQFN];
char fnremote[FMTQFN];
char fnbounce[FMTQFN];

char inbuf[1024];
stralloc sender = {0};

unsigned long id;
datetime_sec qtime;
int flagbounce;
unsigned long size;

unsigned int fmtstats(s)
char *s;
{
 struct datetime dt;
 unsigned int len;
 unsigned int i;

 len = 0;
 datetime_tai(&dt,qtime);
 i = date822fmt(s,&dt) - 7/*XXX*/; len += i; if (s) s += i;
 i = fmt_str(s," GMT  #"); len += i; if (s) s += i;
 i = fmt_ulong(s,id); len += i; if (s) s += i;
 i = fmt_str(s,"  "); len += i; if (s) s += i;
 i = fmt_ulong(s,size); len += i; if (s) s += i;
 i = fmt_str(s,"  <"); len += i; if (s) s += i;
 i = fmt_str(s,sender.s + 1); len += i; if (s) s += i;
 i = fmt_str(s,"> "); len += i; if (s) s += i;
 if (flagbounce)
  {
   i = fmt_str(s," bouncing"); len += i; if (s) s += i;
  }

 return len;
}

stralloc stats = {0};

void out(s,n) char *s; unsigned int n;
{
 while (n > 0)
  {
   substdio_put(subfdout,((*s >= 32) && (*s <= 126)) ? s : "_",1);
   --n;
   ++s;
  }
}
void outs(s) char *s; { out(s,str_len(s)); }
void outok(s) char *s; { substdio_puts(subfdout,s); }

void putstats()
{
 if (!stralloc_ready(&stats,fmtstats(FMT_LEN))) die_nomem();
 stats.len = fmtstats(stats.s);
 out(stats.s,stats.len);
 outok("\n");
}

stralloc line = {0};

void main()
{
 int channel;
 int match;
 struct stat st;
 int fd;
 substdio ss;
 int x;

 if (chdir(auto_qmail) == -1) die_chdir();
 if (chdir("queue") == -1) die_chdir();
 readsubdir_init(&rs,"info",die_opendir);

 while (x = readsubdir_next(&rs,&id))
   if (x > 0)
    {
     fmtqfn(fnmess,"mess/",id,1);
     fmtqfn(fninfo,"info/",id,1);
     fmtqfn(fnlocal,"local/",id,1);
     fmtqfn(fnremote,"remote/",id,1);
     fmtqfn(fnbounce,"bounce/",id,0);

     if (stat(fnmess,&st) == -1) { err(id); continue; }
     size = st.st_size;
     flagbounce = !stat(fnbounce,&st);

     fd = open_read(fninfo);
     if (fd == -1) { err(id); continue; }
     substdio_fdbuf(&ss,read,fd,inbuf,sizeof(inbuf));
     if (getln(&ss,&sender,&match,0) == -1) die_nomem();
     if (fstat(fd,&st) == -1) { close(fd); err(id); continue; }
     close(fd);
     qtime = st.st_mtime;

     putstats();

     for (channel = 0;channel < 2;++channel)
      {
       fd = open_read(channel ? fnremote : fnlocal);
       if (fd == -1)
	{
	 if (errno != error_noent)
	   err(id);
	}
       else
        {
         for (;;)
	  {
	   if (getln(&ss,&line,&match,0) == -1) die_nomem();
	   if (!match) break;
	   switch(line.s[0])
	    {
	     case 'D':
	       outok("  done");
	     case 'T':
	       outok(channel ? "\tremote\t" : "\tlocal\t");
	       outs(line.s + 1);
	       outok("\n");
	       break;
	    }
	  }
         close(fd);
        }
      }
    }

 die(0);
}
