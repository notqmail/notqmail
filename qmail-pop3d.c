#include <sys/types.h>
#include <sys/stat.h>
#include "direntry.h"
#include "sig.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "alloc.h"
#include "datetime.h"
#include "prot.h"
#include "open.h"
#include "prioq.h"
#include "scan.h"
#include "fmt.h"
#include "error.h"
#include "str.h"
#include "exit.h"
#include "now.h"
#include "readwrite.h"

int timeout = 1200;

char ssoutbuf[1024];
substdio ssout = SUBSTDIO_FDBUF(write,1,ssoutbuf,sizeof(ssoutbuf));

int timeoutread(fd,buf,n) int fd; char *buf; int n;
{
 int r; int saveerrno;
 alarm(timeout);
 r = read(fd,buf,n); saveerrno = errno;
 alarm(0);
 errno = saveerrno; return r;
}

char ssinbuf[128];
substdio ssin = SUBSTDIO_FDBUF(timeoutread,0,ssinbuf,sizeof(ssinbuf));


void die() { _exit(0); }
void puts(s) char *s;
{
 if (substdio_puts(&ssout,s) == -1) die();
}
void flush()
{
 if (substdio_flush(&ssout) == -1) die();
}
void err(s) char *s;
{
 puts("-ERR ");
 puts(s);
 puts("\r\n");
 if (substdio_flush(&ssout) == -1) die();
}
void die_nomem() { err("out of memory"); die(); }
void die_prot() { err("protection problem"); die(); }
void die_nomaildir() { err("this user has no $HOME/Maildir"); die(); }

void err_syntax() { err("syntax error"); }
void err_unimpl() { err("unimplemented"); }
void err_deleted() { err("already deleted"); }
void err_nozero() { err("messages are counted from 1"); }
void err_toobig() { err("not that many messages"); }
void err_nosuch() { err("unable to open that message"); }
void err_nounlink() { err("unable to unlink all deleted messages"); }

void okay() { puts("+OK \r\n"); flush(); }
void pop3_last() { puts("+OK 0\r\n"); flush(); }


stralloc dataline = {0};

stralloc filenames = {0};
prioq pq = {0};
stralloc newname = {0};

struct message
 {
  int flagdeleted;
  unsigned long size;
  char *fn;
 }
*m;
int numm;

substdio ssmsg; char ssmsgbuf[1024];


void blast(ssfrom,limit)
substdio *ssfrom;
unsigned long limit;
{
 int match;
 int inheaders = 1;

 for (;;)
  {
   if (getln(ssfrom,&dataline,&match,'\n') != 0) die();
   if (!match && !dataline.len) break;
   if (match) --dataline.len; /* no way to pass this info over POP */
   if (limit) if (!inheaders) if (!--limit) break;
   if (!dataline.len)
     inheaders = 0;
   else
     if (dataline.s[0] == '.')
       substdio_put(&ssout,".",1);
   if (substdio_put(&ssout,dataline.s,dataline.len) == -1) die();
   if (substdio_put(&ssout,"\r\n",2) == -1) die();
   if (!match) break;
  }
 if (substdio_put(&ssout,"\r\n.\r\n",5) == -1) die();
 if (substdio_flush(&ssout) == -1) die();
}

void getlist()
{
 unsigned long pos;
 datetime_sec time;
 DIR *dir;
 direntry *d;
 struct prioq_elt pe;
 struct stat st;
 int i;

 numm = 0;

 time = now();

 if (dir = opendir("tmp"))
  {
   while (d = readdir(dir))
    {
     if (str_equal(d->d_name,".")) continue;
     if (str_equal(d->d_name,"..")) continue;
     if (!stralloc_copys(&newname,"tmp/")) die_nomem();
     if (!stralloc_cats(&newname,d->d_name)) die_nomem();
     if (!stralloc_0(&newname)) die_nomem();
     if (stat(newname.s,&st) == 0)
       if (time > st.st_atime + 129600)
	 unlink(newname.s);
    }
   closedir(dir);
  }

 if (!stralloc_copys(&filenames,"")) die_nomem();

 if (dir = opendir("new"))
  {
   while (d = readdir(dir))
    {
     if (str_equal(d->d_name,".")) continue;
     if (str_equal(d->d_name,"..")) continue;
     pos = filenames.len;
     if (!stralloc_cats(&filenames,"new/")) die_nomem();
     if (!stralloc_cats(&filenames,d->d_name)) die_nomem();
     if (!stralloc_0(&filenames)) die_nomem();
     if (stat(filenames.s + pos,&st) == 0)
      {
       pe.dt = st.st_mtime;
       pe.id = pos;
       if (!prioq_insert(&pq,&pe)) die_nomem();
       ++numm;
      }
    }
   closedir(dir);
  }

 if (dir = opendir("cur"))
  {
   while (d = readdir(dir))
    {
     if (str_equal(d->d_name,".")) continue;
     if (str_equal(d->d_name,"..")) continue;
     pos = filenames.len;
     if (!stralloc_cats(&filenames,"cur/")) die_nomem();
     if (!stralloc_cats(&filenames,d->d_name)) die_nomem();
     if (!stralloc_0(&filenames)) die_nomem();
     if (stat(filenames.s + pos,&st) == 0)
      {
       pe.dt = st.st_mtime;
       pe.id = pos;
       if (!prioq_insert(&pq,&pe)) die_nomem();
       ++numm;
      }
    }
   closedir(dir);
  }

 m = (struct message *) alloc(numm * sizeof(struct message));
 if (!m) die_nomem();

 for (i = 0;i < numm;++i)
  {
   if (!prioq_min(&pq,&pe)) { numm = i; break; }
   prioq_delmin(&pq);
   m[i].fn = filenames.s + pe.id;
   m[i].flagdeleted = 0;
   if (stat(m[i].fn,&st) == -1)
     m[i].size = 0;
   else
     m[i].size = st.st_size;
  }
}

char foo[FMT_ULONG];

void printint(u) unsigned int u;
{
 foo[fmt_uint(foo,u)] = 0;
 puts(foo);
 puts(" ");
}

void printlong(u) unsigned long u;
{
 foo[fmt_uint(foo,u)] = 0;
 puts(foo);
 puts("\r\n");
}

void printfn(fn) char *fn;
{
 puts(fn + 4);
 puts("\r\n");
}

void pop3_stat()
{
 int i;
 unsigned long total;

 total = 0;
 for (i = 0;i < numm;++i) if (!m[i].flagdeleted) total += m[i].size;
 puts("+OK ");
 printint(numm);
 printlong(total);
 flush();
}

void pop3_rset()
{
 int i;
 for (i = 0;i < numm;++i) m[i].flagdeleted = 0;
 okay();
}

void pop3_quit()
{
 int i;
 for (i = 0;i < numm;++i)
   if (m[i].flagdeleted)
     if (unlink(m[i].fn) == -1) err_nounlink();
 okay();
 die();
}

int msgno(arg) char *arg;
{
 unsigned long u;
 if (!arg) { err_syntax(); return -1; }
 if (!scan_ulong(arg,&u)) { err_syntax(); return -1; }
 if (!u) { err_nozero(); return -1; }
 --u;
 if (u >= numm) { err_toobig(); return -1; }
 if (m[u].flagdeleted) { err_deleted(); return -1; }
 return u;
}

void pop3_dele(arg) char *arg;
{
 int i;

 i = msgno(arg);
 if (i == -1) return;
 m[i].flagdeleted = 1;
 okay();
}

void dolisting(arg,flaguidl) char *arg; int flaguidl;
{
 unsigned int i;

 if (arg)
  {
   i = msgno(arg);
   if (i == -1) return;
   puts("+OK ");
   printint(i + 1);
   if (flaguidl) printfn(m[i].fn); else printlong(m[i].size);
  }
 else
  {
   okay();

   for (i = 0;i < numm;++i)
     if (!m[i].flagdeleted)
      {
       printint(i + 1);
       if (flaguidl) printfn(m[i].fn); else printlong(m[i].size);
      }
   puts(".\r\n");
  }
 flush();
}

void pop3_uidl(arg) char *arg; { dolisting(arg,1); }
void pop3_list(arg) char *arg; { dolisting(arg,0); }

void pop3_top(arg) char *arg;
{
 int i;
 unsigned long limit;
 int fd;

 i = msgno(arg);
 if (i == -1) return;

 arg += scan_ulong(arg,&limit);
 while (*arg == ' ') ++arg;
 if (scan_ulong(arg,&limit)) ++limit; else limit = 0;

 fd = open_read(m[i].fn);
 if (fd == -1) { err_nosuch(); return; }
 okay();
 substdio_fdbuf(&ssmsg,read,fd,ssmsgbuf,sizeof(ssmsgbuf));
 blast(&ssmsg,limit);
 close(fd);
}

static struct { void (*fun)(); char *text; } pop3cmd[] = {
  { pop3_quit, "quit" }
, { pop3_stat, "stat" }
, { pop3_list, "list" }
, { pop3_uidl, "uidl" }
, { pop3_dele, "dele" }
, { pop3_top, "retr" }
, { pop3_rset, "rset" }
, { pop3_last, "last" }
, { pop3_top, "top" }
, { okay, "noop" }
, { 0, 0 }
};

void doit(cmd)
char *cmd;
{
 int i;
 int j;
 char ch;

 for (i = 0;pop3cmd[i].fun;++i)
  {
   for (j = 0;ch = pop3cmd[i].text[j];++j)
     if ((cmd[j] != ch) && (cmd[j] != ch - 32))
       break;
   if (!ch)
     if (!cmd[j] || (cmd[j] == ' '))
      {
       while (cmd[j] == ' ') ++j;
       if (!cmd[j])
         pop3cmd[i].fun((char *) 0);
       else
         pop3cmd[i].fun(cmd + j);
       return;
      }
  }
 err_unimpl();
}

void main(argc,argv)
int argc;
char **argv;
{
 static stralloc cmd = {0};
 int match;

 sig_alarmcatch(die);
 sig_pipeignore();

 if (!argv[1]) die_nomaildir();
 if (chdir(argv[1]) == -1) die_nomaildir();

 getlist();

 okay();

 for (;;)
  {
   if (getln(&ssin,&cmd,&match,'\n') == -1) die();
   if (!match) die();
   if (cmd.len == 0) die();
   if (cmd.s[--cmd.len] != '\n') die();
   if ((cmd.len > 0) && (cmd.s[cmd.len - 1] == '\r')) --cmd.len;
   cmd.s[cmd.len++] = 0;
   doit(cmd.s);
  }
}
