#include <sys/types.h>
#include <sys/stat.h>
#include "alloc.h"
#include "auto_qmail.h"
#include "byte.h"
#include "constmap.h"
#include "control.h"
#include "direntry.h"
#include "error.h"
#include "exit.h"
#include "fmt.h"
#include "fmtqfn.h"
#include "getln.h"
#include "open.h"
#include "ndelay.h"
#include "now.h"
#include "readsubdir.h"
#include "readwrite.h"
#include "scan.h"
#include "select.h"
#include "str.h"
#include "stralloc.h"
#include "substdio.h"
#include "trigger.h"

/* critical timing feature #1: if not triggered, do not busy-loop */
/* critical timing feature #2: if triggered, respond within fixed time */
/* important timing feature: when triggered, respond instantly */
#define SLEEP_TODO 1500 /* check todo/ every 25 minutes in any case */
#define SLEEP_FUZZ 1 /* slop a bit on sleeps to avoid zeno effect */
#define SLEEP_FOREVER 86400 /* absolute maximum time spent in select() */
#define SLEEP_SYSFAIL 123

stralloc percenthack = {0};
struct constmap mappercenthack;
stralloc locals = {0};
struct constmap maplocals;
stralloc vdoms = {0};
struct constmap mapvdoms;
stralloc envnoathost = {0};

char strnum[FMT_ULONG];

/* XXX not good, if qmail-send.c changes this has to be updated */
#define CHANNELS 2
char *chanaddr[CHANNELS] = { "local/", "remote/" };

datetime_sec recent;

void log1(char *x);
void log3(char* x, char* y, char* z);

int flagstopasap = 0;
void sigterm(void)
{
  if (flagstopasap == 0)
    log1("status: qmail-todo stop processing asap\n");
  flagstopasap = 1;
}

int flagreadasap = 0; void sighup(void) { flagreadasap = 1; }
int flagsendalive = 1; void senddied(void) { flagsendalive = 0; }

void nomem() { log1("alert: out of memory, sleeping...\n"); sleep(10); }
void pausedir(dir) char *dir;
{ log3("alert: unable to opendir ",dir,", sleeping...\n"); sleep(10); }

void cleandied()
{ 
  log1("alert: qmail-todo: oh no! lost qmail-clean connection! dying...\n");
  flagstopasap = 1;
}


/* this file is not so long ------------------------------------- FILENAMES */

stralloc fn = {0};

void fnmake_init(void)
{
 while (!stralloc_ready(&fn,FMTQFN)) nomem();
}

void fnmake_info(unsigned long id) { fn.len = fmtqfn(fn.s,"info/",id,1); }
void fnmake_todo(unsigned long id) { fn.len = fmtqfn(fn.s,"todo/",id,0); }
void fnmake_mess(unsigned long id) { fn.len = fmtqfn(fn.s,"mess/",id,1); }
void fnmake_chanaddr(unsigned long id, int c)
{ fn.len = fmtqfn(fn.s,chanaddr[c],id,1); }


/* this file is not so long ------------------------------------- REWRITING */

stralloc rwline = {0};

/* 1 if by land, 2 if by sea, 0 if out of memory. not allowed to barf. */
/* may trash recip. must set up rwline, between a T and a \0. */
int rewrite(char *recip)
{
  int i;
  int j;
  char *x;
  static stralloc addr = {0};
  int at;

  if (!stralloc_copys(&rwline,"T")) return 0;
  if (!stralloc_copys(&addr,recip)) return 0;

  i = byte_rchr(addr.s,addr.len,'@');
  if (i == addr.len) {
    if (!stralloc_cats(&addr,"@")) return 0;
    if (!stralloc_cat(&addr,&envnoathost)) return 0;
  }

  while (constmap(&mappercenthack,addr.s + i + 1,addr.len - i - 1)) {
    j = byte_rchr(addr.s,i,'%');
    if (j == i) break;
    addr.len = i;
    i = j;
    addr.s[i] = '@';
  }

  at = byte_rchr(addr.s,addr.len,'@');

  if (constmap(&maplocals,addr.s + at + 1,addr.len - at - 1)) {
    if (!stralloc_cat(&rwline,&addr)) return 0;
    if (!stralloc_0(&rwline)) return 0;
    return 1;
  }

  for (i = 0;i <= addr.len;++i)
    if (!i || (i == at + 1) || (i == addr.len) || ((i > at) && (addr.s[i] == '.')))
      if (x = constmap(&mapvdoms,addr.s + i,addr.len - i)) {
        if (!*x) break;
        if (!stralloc_cats(&rwline,x)) return 0;
        if (!stralloc_cats(&rwline,"-")) return 0;
        if (!stralloc_cat(&rwline,&addr)) return 0;
        if (!stralloc_0(&rwline)) return 0;
        return 1;
      }
 
  if (!stralloc_cat(&rwline,&addr)) return 0;
  if (!stralloc_0(&rwline)) return 0;
  return 2;
}

/* this file is not so long --------------------------------- COMMUNICATION */

substdio sstoqc; char sstoqcbuf[1024];
substdio ssfromqc; char ssfromqcbuf[1024];
stralloc comm_buf = {0};
int comm_pos;
int fdout = -1;
int fdin = -1;

void comm_init(void)
{
 substdio_fdbuf(&sstoqc,write,2,sstoqcbuf,sizeof(sstoqcbuf));
 substdio_fdbuf(&ssfromqc,read,3,ssfromqcbuf,sizeof(ssfromqcbuf));

 fdout = 1; /* stdout */
 fdin = 0;  /* stdin */
 if (ndelay_on(fdout) == -1)
 /* this is so stupid: NDELAY semantics should be default on write */
   senddied(); /* drastic, but better than risking deadlock */

 while (!stralloc_ready(&comm_buf,1024)) nomem();
}

int comm_canwrite(void)
{
 /* XXX: could allow a bigger buffer; say 10 recipients */
 /* XXX: returns true if there is something in the buffer */
 if (!flagsendalive) return 0;
 if (comm_buf.s && comm_buf.len) return 1;
 return 0;
}

void log1(char* x)
{
  int pos;
  
  pos = comm_buf.len;
  if (!stralloc_cats(&comm_buf,"L")) goto fail;
  if (!stralloc_cats(&comm_buf,x)) goto fail;
  if (!stralloc_0(&comm_buf)) goto fail;
  return;
  
fail:
  /* either all or nothing */
  comm_buf.len = pos;
}

void log3(char* x, char *y, char *z)
{
  int pos;
  
  pos = comm_buf.len;
  if (!stralloc_cats(&comm_buf,"L")) goto fail;
  if (!stralloc_cats(&comm_buf,x)) goto fail;
  if (!stralloc_cats(&comm_buf,y)) goto fail;
  if (!stralloc_cats(&comm_buf,z)) goto fail;
  if (!stralloc_0(&comm_buf)) goto fail;
  return;
  
fail:
  /* either all or nothing */
  comm_buf.len = pos;
}

void comm_write(unsigned long id, int local, int remote)
{
  int pos;
  char *s;
  
  if(local && remote) s="B";
  else if(local) s="L";
  else if(remote) s="R";
  else s="X";
  
  pos = comm_buf.len;
  strnum[fmt_ulong(strnum,id)] = 0;
  if (!stralloc_cats(&comm_buf,"D")) goto fail;
  if (!stralloc_cats(&comm_buf,s)) goto fail;
  if (!stralloc_cats(&comm_buf,strnum)) goto fail;
  if (!stralloc_0(&comm_buf)) goto fail;
  return;
  
fail:
  /* either all or nothing */
  comm_buf.len = pos;
}

static int issafe(char ch)
{
 if (ch == '%') return 0; /* general principle: allman's code is crap */
 if (ch < 33) return 0;
 if (ch > 126) return 0;
 return 1;
}

void comm_info(unsigned long id, unsigned long size, char* from, unsigned long pid, unsigned long uid)
{
  int pos;
  int i;
  
  pos = comm_buf.len;
  if (!stralloc_cats(&comm_buf,"Linfo msg ")) goto fail;
  strnum[fmt_ulong(strnum,id)] = 0;
  if (!stralloc_cats(&comm_buf,strnum)) goto fail;
  if (!stralloc_cats(&comm_buf,": bytes ")) goto fail;
  strnum[fmt_ulong(strnum,size)] = 0;
  if (!stralloc_cats(&comm_buf,strnum)) goto fail;
  if (!stralloc_cats(&comm_buf," from <")) goto fail;
  i = comm_buf.len;
  if (!stralloc_cats(&comm_buf,from)) goto fail;
  for (;i < comm_buf.len;++i)
    if (comm_buf.s[i] == '\n')
      comm_buf.s[i] = '/';
    else
      if (!issafe(comm_buf.s[i]))
	comm_buf.s[i] = '_';
  if (!stralloc_cats(&comm_buf,"> qp ")) goto fail;
  strnum[fmt_ulong(strnum,pid)] = 0;
  if (!stralloc_cats(&comm_buf,strnum)) goto fail;
  if (!stralloc_cats(&comm_buf," uid ")) goto fail;
  strnum[fmt_ulong(strnum,uid)] = 0;
  if (!stralloc_cats(&comm_buf,strnum)) goto fail;
  if (!stralloc_cats(&comm_buf,"\n")) goto fail;
  if (!stralloc_0(&comm_buf)) goto fail;
  return;
  
fail:
  /* either all or nothing */
  comm_buf.len = pos;
}

void comm_exit(void)
{
  int w;
  
  /* if it fails exit, we have already stoped */
  if (!stralloc_cats(&comm_buf,"X")) _exit(1);
  if (!stralloc_0(&comm_buf)) _exit(1);
}

void comm_selprep(int *nfds, fd_set *wfds, fd_set *rfds)
{
  if (flagsendalive) {
    if (flagstopasap && comm_canwrite() == 0)
      comm_exit();
    if (comm_canwrite()) {
      FD_SET(fdout,wfds);
      if (*nfds <= fdout)
	*nfds = fdout + 1;
    }
    FD_SET(fdin,rfds);
    if (*nfds <= fdin)
      *nfds = fdin + 1;
  }
}

void comm_do(fd_set *wfds, fd_set *rfds)
{
  /* first write then read */
  if (flagsendalive)
    if (comm_canwrite())
      if (FD_ISSET(fdout,wfds)) {
	int w;
	int len;
	len = comm_buf.len;
	w = write(fdout,comm_buf.s + comm_pos,len - comm_pos);
	if (w <= 0) {
	  if ((w == -1) && (errno == error_pipe))
	    senddied();
	} else {
	  comm_pos += w;
	  if (comm_pos == len) {
	    comm_buf.len = 0;
	    comm_pos = 0;
	  }
	}
      }
  if (flagsendalive)
    if (FD_ISSET(fdin,rfds)) {
      /* there are only two messages 'H' and 'X' */
      char c;
      int r;
      r = read(fdin, &c, 1);
      if (r <= 0) {
	if ((r == -1) && (errno != error_intr))
	  senddied();
      } else {
	switch(c) {
	  case 'H':
	    sighup();
	    break;
	  case 'X':
	    sigterm();
	    break;
	  default:
	    log1("warning: qmail-todo: qmail-send speaks an obscure dialect\n");
	    break;
	}
      }
    }
}

/* this file is not so long ------------------------------------------ TODO */

datetime_sec nexttodorun;
DIR *tododir; /* if 0, have to opendir again */
stralloc todoline = {0};
char todobuf[SUBSTDIO_INSIZE];
char todobufinfo[512];
char todobufchan[CHANNELS][1024];

void todo_init(void)
{
 tododir = 0;
 nexttodorun = now();
 trigger_set();
}

void todo_selprep(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
 if (flagstopasap) return;
 trigger_selprep(nfds,rfds);
 if (tododir) *wakeup = 0;
 if (*wakeup > nexttodorun) *wakeup = nexttodorun;
}

void todo_do(fd_set *rfds)
{
 struct stat st;
 substdio ss; int fd;
 substdio ssinfo; int fdinfo;
 substdio sschan[CHANNELS];
 int fdchan[CHANNELS];
 int flagchan[CHANNELS];
 char ch;
 int match;
 unsigned long id;
 unsigned int len;
 direntry *d;
 int c;
 unsigned long uid;
 unsigned long pid;

 fd = -1;
 fdinfo = -1;
 for (c = 0;c < CHANNELS;++c) fdchan[c] = -1;

 if (flagstopasap) return;

 if (!tododir)
  {
   if (!trigger_pulled(rfds))
     if (recent < nexttodorun)
       return;
   trigger_set();
   tododir = opendir("todo");
   if (!tododir)
    {
     pausedir("todo");
     return;
    }
   nexttodorun = recent + SLEEP_TODO;
  }

 d = readdir(tododir);
 if (!d)
  {
   closedir(tododir);
   tododir = 0;
   return;
  }
 if (str_equal(d->d_name,".")) return;
 if (str_equal(d->d_name,"..")) return;
 len = scan_ulong(d->d_name,&id);
 if (!len || d->d_name[len]) return;

 fnmake_todo(id);

 fd = open_read(fn.s);
 if (fd == -1) { log3("warning: qmail-todo: unable to open ",fn.s,"\n"); return; }

 fnmake_mess(id);
 /* just for the statistics */
 if (stat(fn.s,&st) == -1)
  { log3("warning: qmail-todo: unable to stat ",fn.s,"\n"); goto fail; }

 for (c = 0;c < CHANNELS;++c)
  {
   fnmake_chanaddr(id,c);
   if (unlink(fn.s) == -1) if (errno != error_noent)
    { log3("warning: qmail-todo: unable to unlink ",fn.s,"\n"); goto fail; }
  }

 fnmake_info(id);
 if (unlink(fn.s) == -1) if (errno != error_noent)
  { log3("warning: qmail-todo: unable to unlink ",fn.s,"\n"); goto fail; }

 fdinfo = open_excl(fn.s);
 if (fdinfo == -1)
  { log3("warning: qmail-todo: unable to create ",fn.s,"\n"); goto fail; }

 strnum[fmt_ulong(strnum,id)] = 0;
 log3("new msg ",strnum,"\n");

 for (c = 0;c < CHANNELS;++c) flagchan[c] = 0;

 substdio_fdbuf(&ss,read,fd,todobuf,sizeof(todobuf));
 substdio_fdbuf(&ssinfo,write,fdinfo,todobufinfo,sizeof(todobufinfo));

 uid = 0;
 pid = 0;

 for (;;)
  {
   if (getln(&ss,&todoline,&match,'\0') == -1)
    {
     /* perhaps we're out of memory, perhaps an I/O error */
     fnmake_todo(id);
     log3("warning: qmail-todo: trouble reading ",fn.s,"\n"); goto fail;
    }
   if (!match) break;

   switch(todoline.s[0])
    {
     case 'u':
       scan_ulong(todoline.s + 1,&uid);
       break;
     case 'p':
       scan_ulong(todoline.s + 1,&pid);
       break;
     case 'F':
       if (substdio_putflush(&ssinfo,todoline.s,todoline.len) == -1)
	{
	 fnmake_info(id);
         log3("warning: qmail-todo: trouble writing to ",fn.s,"\n"); goto fail;
	}
	comm_info(id, (unsigned long) st.st_size, todoline.s + 1, pid, uid);
       break;
     case 'T':
       switch(rewrite(todoline.s + 1))
	{
	 case 0: nomem(); goto fail;
	 case 2: c = 1; break;
	 default: c = 0; break;
        }
       if (fdchan[c] == -1)
	{
	 fnmake_chanaddr(id,c);
	 fdchan[c] = open_excl(fn.s);
	 if (fdchan[c] == -1)
          { log3("warning: qmail-todo: unable to create ",fn.s,"\n"); goto fail; }
	 substdio_fdbuf(&sschan[c]
	   ,write,fdchan[c],todobufchan[c],sizeof(todobufchan[c]));
	 flagchan[c] = 1;
	}
       if (substdio_bput(&sschan[c],rwline.s,rwline.len) == -1)
        {
	 fnmake_chanaddr(id,c);
         log3("warning: qmail-todo: trouble writing to ",fn.s,"\n"); goto fail;
        }
       break;
     default:
       fnmake_todo(id);
       log3("warning: qmail-todo: unknown record type in ",fn.s,"\n"); goto fail;
    }
  }

 close(fd); fd = -1;

 fnmake_info(id);
 if (substdio_flush(&ssinfo) == -1)
  { log3("warning: qmail-todo: trouble writing to ",fn.s,"\n"); goto fail; }
 if (fsync(fdinfo) == -1)
  { log3("warning: qmail-todo: trouble fsyncing ",fn.s,"\n"); goto fail; }
 close(fdinfo); fdinfo = -1;

 for (c = 0;c < CHANNELS;++c)
   if (fdchan[c] != -1)
    {
     fnmake_chanaddr(id,c);
     if (substdio_flush(&sschan[c]) == -1)
      { log3("warning: qmail-todo: trouble writing to ",fn.s,"\n"); goto fail; }
     if (fsync(fdchan[c]) == -1)
      { log3("warning: qmail-todo: trouble fsyncing ",fn.s,"\n"); goto fail; }
     close(fdchan[c]); fdchan[c] = -1;
    }

 fnmake_todo(id);
 if (substdio_putflush(&sstoqc,fn.s,fn.len) == -1) { cleandied(); return; }
 if (substdio_get(&ssfromqc,&ch,1) != 1) { cleandied(); return; }
 if (ch != '+')
  {
   log3("warning: qmail-clean unable to clean up ",fn.s,"\n");
   return;
  }

 comm_write(id, flagchan[0], flagchan[1]);
 
 return;
 
 fail:
 if (fd != -1) close(fd);
 if (fdinfo != -1) close(fdinfo);
 for (c = 0;c < CHANNELS;++c)
   if (fdchan[c] != -1) close(fdchan[c]);
}

/* this file is too long ---------------------------------------------- MAIN */

int getcontrols(void)
{
 if (control_init() == -1) return 0;
 if (control_rldef(&envnoathost,"control/envnoathost",1,"envnoathost") != 1) return 0;
 if (control_readfile(&locals,"control/locals",1) != 1) return 0;
 if (!constmap_init(&maplocals,locals.s,locals.len,0)) return 0;
 switch(control_readfile(&percenthack,"control/percenthack",0))
  {
   case -1: return 0;
   case 0: if (!constmap_init(&mappercenthack,"",0,0)) return 0; break;
   case 1: if (!constmap_init(&mappercenthack,percenthack.s,percenthack.len,0)) return 0; break;
  }
 switch(control_readfile(&vdoms,"control/virtualdomains",0))
  {
   case -1: return 0;
   case 0: if (!constmap_init(&mapvdoms,"",0,1)) return 0; break;
   case 1: if (!constmap_init(&mapvdoms,vdoms.s,vdoms.len,1)) return 0; break;
  }
 return 1;
}

stralloc newlocals = {0};
stralloc newvdoms = {0};

void regetcontrols(void)
{
 int r;

 if (control_readfile(&newlocals,"control/locals",1) != 1)
  { log1("alert: qmail-todo: unable to reread control/locals\n"); return; }
 r = control_readfile(&newvdoms,"control/virtualdomains",0);
 if (r == -1)
  { log1("alert: qmail-todo: unable to reread control/virtualdomains\n"); return; }

 constmap_free(&maplocals);
 constmap_free(&mapvdoms);

 while (!stralloc_copy(&locals,&newlocals)) nomem();
 while (!constmap_init(&maplocals,locals.s,locals.len,0)) nomem();

 if (r)
  {
   while (!stralloc_copy(&vdoms,&newvdoms)) nomem();
   while (!constmap_init(&mapvdoms,vdoms.s,vdoms.len,1)) nomem();
  }
 else
   while (!constmap_init(&mapvdoms,"",0,1)) nomem();
}

void reread(void)
{
 if (chdir(auto_qmail) == -1)
  {
   log1("alert: qmail-todo: unable to reread controls: unable to switch to home directory\n");
   return;
  }
 regetcontrols();
 while (chdir("queue") == -1)
  {
   log1("alert: qmail-todo: unable to switch back to queue directory; HELP! sleeping...\n");
   sleep(10);
  }
}

void main()
{
 datetime_sec wakeup;
 fd_set rfds;
 fd_set wfds;
 int nfds;
 struct timeval tv;
 int r;
 char c;

 if (chdir(auto_qmail) == -1)
  { log1("alert: qmail-todo: cannot start: unable to switch to home directory\n"); _exit(111); }
 if (!getcontrols())
  { log1("alert: qmail-todo: cannot start: unable to read controls\n"); _exit(111); }
 if (chdir("queue") == -1)
  { log1("alert: qmail-todo: cannot start: unable to switch to queue directory\n"); _exit(111); }
 sig_pipeignore();
 umask(077);

 fnmake_init();

 todo_init();
 comm_init();
 
 do {
   r = read(fdin, &c, 1);
   if ((r == -1) && (errno != error_intr))
     _exit(100); /* read failed probably qmail-send died */
 } while (r =! 1); /* we assume it is a 'S' */
 
 for (;;)
  {
   recent = now();

   if (flagreadasap) { flagreadasap = 0; reread(); }
   if (!flagsendalive) {
     /* qmail-send finaly exited, so do the same. */
     if (flagstopasap) _exit(0);
     /* qmail-send died. We can not log and we can not work therefor _exit(1). */
     _exit(1);
   }

   wakeup = recent + SLEEP_FOREVER;
   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   nfds = 1;

   todo_selprep(&nfds,&rfds,&wakeup);
   comm_selprep(&nfds,&wfds,&rfds);

   if (wakeup <= recent) tv.tv_sec = 0;
   else tv.tv_sec = wakeup - recent + SLEEP_FUZZ;
   tv.tv_usec = 0;

   if (select(nfds,&rfds,&wfds,(fd_set *) 0,&tv) == -1)
     if (errno == error_intr)
       ;
     else
       log1("warning: qmail-todo: trouble in select\n");
   else
    {
     recent = now();

     todo_do(&rfds);
     comm_do(&wfds, &rfds);
    }
  }
  /* NOTREACHED */
}

