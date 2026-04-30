#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include "commands.h"
#include "sig.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "alloc.h"
#include "open.h"
#include "prioq.h"
#include "scan.h"
#include "fmt.h"
#include "str.h"
#include "exit.h"
#include "maildir.h"
#include "readwrite.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

void die() { _exit(0); }

extern int rename(const char *, const char *);

GEN_SAFE_TIMEOUTREAD(saferead,1200,fd,die())
GEN_SAFE_TIMEOUTWRITE(safewrite,1200,fd,die())

char sserrbuf[128];
substdio sserr = SUBSTDIO_FDBUF(safewrite,2,sserrbuf,sizeof(sserrbuf));

char ssoutbuf[1024];
substdio ssout = SUBSTDIO_FDBUF(safewrite,1,ssoutbuf,sizeof(ssoutbuf));

char ssinbuf[128];
substdio ssin = SUBSTDIO_FDBUF(saferead,0,ssinbuf,sizeof(ssinbuf));

void put(char *buf, int len)
{
  substdio_put(&ssout,buf,len);
}
void puts(char *s)
{
  substdio_puts(&ssout,s);
}
void flush(void)
{
  substdio_flush(&ssout);
}
void err(char *s)
{
  puts("-ERR ");
  puts(s);
  puts("\r\n");
  flush();
}

void die_nomem(void) { err("out of memory"); die(); }
void die_nomaildir(void) { err("this user has no $HOME/Maildir"); die(); }
void die_root(void) {
  substdio_putsflush(&sserr,"qmail-pop3d invoked as uid 0, terminating\n");
  _exit(1);
}
void die_scan(void) { err("unable to scan $HOME/Maildir"); die(); }

void err_syntax(void) { err("syntax error"); }
void err_unimpl(char *arg, void *data) { err("unimplemented"); }
void err_deleted(void) { err("already deleted"); }
void err_nozero(void) { err("messages are counted from 1"); }
void err_toobig(void) { err("not that many messages"); }
void err_nosuch(void) { err("unable to open that message"); }
void err_nounlink(void) { err("unable to unlink all deleted messages"); }

void okay(char *arg, void *data) { puts("+OK \r\n"); flush(); }

void printfn(char *fn)
{
  fn += 4;
  put(fn,str_chr(fn,':'));
}

char strnum[FMT_ULONG];
stralloc line = {0};

void blast(substdio *ssfrom, unsigned long limit)
{
  int match;
  int inheaders = 1;
 
  for (;;) {
    if (getln(ssfrom,&line,&match,'\n') != 0) die();
    if (!match && !line.len) break;
    if (match) --line.len; /* no way to pass this info over POP */
    if (limit) if (!inheaders) if (!--limit) break;
    if (!line.len)
      inheaders = 0;
    else
      if (line.s[0] == '.')
        put(".",1);
    put(line.s,line.len);
    put("\r\n",2);
    if (!match) break;
  }
  put("\r\n.\r\n",5);
  flush();
}

stralloc filenames = {0};
prioq pq = {0};

struct message {
  int flagdeleted;
  unsigned long size;
  char *fn;
} *m;
unsigned int numm;

int last = 0;

void getlist(void)
{
  struct prioq_elt pe;
  struct stat st;
  unsigned int i;
 
  maildir_clean(&line);
  if (maildir_scan(&pq,&filenames,1,1) == -1) die_scan();
 
  numm = pq.p ? pq.len : 0;
  m = calloc(numm, sizeof(struct message));
  if (!m) die_nomem();
 
  for (i = 0;i < numm;++i) {
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

void pop3_stat(char *arg, void *data)
{
  unsigned int i;
  unsigned long total;
 
  total = 0;
  for (i = 0;i < numm;++i) if (!m[i].flagdeleted) total += m[i].size;
  puts("+OK ");
  put(strnum,fmt_uint(strnum,numm));
  puts(" ");
  put(strnum,fmt_ulong(strnum,total));
  puts("\r\n");
  flush();
}

void pop3_rset(char *arg, void *data)
{
  unsigned int i;
  for (i = 0;i < numm;++i) m[i].flagdeleted = 0;
  last = 0;
  okay(0, 0);
}

void pop3_last(char *arg, void *data)
{
  puts("+OK ");
  put(strnum,fmt_uint(strnum,last));
  puts("\r\n");
  flush();
}

void pop3_quit(char *arg, void *data)
{
  unsigned int i;
  for (i = 0;i < numm;++i)
    if (m[i].flagdeleted) {
      if (unlink(m[i].fn) == -1) err_nounlink();
    }
    else
      if (str_start(m[i].fn,"new/")) {
	if (!stralloc_copys(&line,"cur/")) die_nomem();
	if (!stralloc_cats(&line,m[i].fn + 4)) die_nomem();
	if (!stralloc_cats(&line,":2,")) die_nomem();
	if (!stralloc_0(&line)) die_nomem();
	rename(m[i].fn,line.s); /* if it fails, bummer */
      }
  okay(0, 0);
  die();
}

int msgno(char *arg)
{
  unsigned long u;
  if (!scan_ulong(arg,&u)) { err_syntax(); return -1; }
  if (!u) { err_nozero(); return -1; }
  --u;
  if (u >= numm || u >= INT_MAX) { err_toobig(); return -1; }
  if (m[u].flagdeleted) { err_deleted(); return -1; }
  return u;
}

void pop3_dele(char *arg, void *data)
{
  int i;
  i = msgno(arg);
  if (i == -1) return;
  m[i].flagdeleted = 1;
  if (i + 1 > last) last = i + 1;
  okay(0, 0);
}

void list(int i, int flaguidl)
{
  put(strnum,fmt_uint(strnum,i + 1));
  puts(" ");
  if (flaguidl) printfn(m[i].fn);
  else put(strnum,fmt_ulong(strnum,m[i].size));
  puts("\r\n");
}

void dolisting(char *arg, int flaguidl)
{
  unsigned int i;
  if (*arg) {
    i = msgno(arg);
    if (i == -1) return;
    puts("+OK ");
    list(i,flaguidl);
  }
  else {
    okay(0, 0);
    for (i = 0;i < numm;++i)
      if (!m[i].flagdeleted)
	list(i,flaguidl);
    puts(".\r\n");
  }
  flush();
}

void pop3_uidl(char *arg, void *data) { dolisting(arg,1); }
void pop3_list(char *arg, void *data) { dolisting(arg,0); }

substdio ssmsg; char ssmsgbuf[1024];

void pop3_top(char *arg, void *data)
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
  okay(0, 0);
  substdio_fdbuf(&ssmsg,read,fd,ssmsgbuf,sizeof(ssmsgbuf));
  blast(&ssmsg,limit);
  close(fd);
}

struct command pop3commands[] = {
  { "quit", pop3_quit, 0, 0 }
, { "stat", pop3_stat, 0, 0 }
, { "list", pop3_list, 0, 0 }
, { "uidl", pop3_uidl, 0, 0 }
, { "dele", pop3_dele, 0, 0 }
, { "retr", pop3_top, 0, 0 }
, { "rset", pop3_rset, 0, 0 }
, { "last", pop3_last, 0, 0 }
, { "top", pop3_top, 0, 0 }
, { "noop", okay, 0, 0 }
, { 0, err_unimpl, 0, 0 }
} ;

int main(int argc, char **argv)
{
  stralloc line = {0};
  sig_alarmcatch(die);
  sig_pipeignore();
 
  if (!getuid()) die_root();
  if (!argv[1]) die_nomaildir();
  if (chdir(argv[1]) == -1) die_nomaildir();
 
  getlist();

  okay(0, 0);
  for (;;) {
    line.len = 0;

    for (;;) {
      int j;
      if (!stralloc_readyplus(&line, 1))
        die_nomem();
      j = substdio_get(&ssin, line.s + line.len, 1);
      if (j == 0)
        die();
      if (line.s[line.len] == '\n') break;
      ++line.len;
    }
    execute_command(line.s, line.len, pop3commands);
  }
}
