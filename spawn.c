#include <sys/types.h>
#include <sys/stat.h>
#include "sig.h"
#include "wait.h"
#include "substdio.h"
#include "byte.h"
#include "str.h"
#include "stralloc.h"
#include "select.h"
#include "exit.h"
#include "coe.h"
#include "open.h"
#include "error.h"
#include "auto_qmail.h"
#include "auto_uids.h"
#include "auto_spawn.h"

extern int truncreport;
extern int spawn();
extern void report();
extern void initialize();

struct delivery
 {
  int used;
  int fdin; /* pipe input */
  int pid; /* zero if child is dead */
  int wstat; /* if !pid: status of child */
  int fdout; /* pipe output, -1 if !pid; delays eof until after death */
  stralloc output;
 }
;

struct delivery *d;

void sigchld()
{
 int wstat;
 int pid;
 int i;
 while ((pid = wait_nohang(&wstat)) > 0)
   for (i = 0;i < auto_spawn;++i) if (d[i].used)
     if (d[i].pid == pid)
      {
       close(d[i].fdout); d[i].fdout = -1;
       d[i].wstat = wstat; d[i].pid = 0;
      }
}

int flagwriting = 1;

int okwrite(fd,buf,n) int fd; char *buf; int n;
{
 int w;
 if (!flagwriting) return n;
 w = write(fd,buf,n);
 if (w != -1) return w;
 if (errno == error_intr) return -1;
 flagwriting = 0; close(fd);
 return n;
}

int flagreading = 1;
char outbuf[1024]; substdio ssout;

int stage = 0; /* reading 0:delnum 1:messid 2:sender 3:recip */
int flagabort = 0; /* if 1, everything except delnum is garbage */
int delnum;
stralloc messid = {0};
stralloc sender = {0};
stralloc recip = {0};

void err(s) char *s;
{
 char ch; ch = delnum; substdio_put(&ssout,&ch,1);
 substdio_puts(&ssout,s); substdio_putflush(&ssout,"",1);
}

void docmd()
{
 int f;
 int i;
 int j;
 int fdmess;
 int pi[2];
 struct stat st;

 if (flagabort) { err("Zqmail-spawn out of memory. (#4.3.0)\n"); return; }
 if (delnum < 0) { err("ZInternal error: delnum negative. (#4.3.5)\n"); return; }
 if (delnum >= auto_spawn) { err("ZInternal error: delnum too big. (#4.3.5)\n"); return; }
 if (d[delnum].used) { err("ZInternal error: delnum in use. (#4.3.5)\n"); return; }
 for (i = 0;i < messid.len;++i)
   if (messid.s[i])
     if (!i || (messid.s[i] != '/'))
       if ((unsigned char) (messid.s[i] - '0') > 9)
        { err("DInternal error: messid has nonnumerics. (#5.3.5)\n"); return; }
 if (messid.len > 100) { err("DInternal error: messid too long. (#5.3.5)\n"); return; }
 if (!messid.s[0]) { err("DInternal error: messid too short. (#5.3.5)\n"); return; }

 if (!stralloc_copys(&d[delnum].output,""))
  { err("Zqmail-spawn out of memory. (#4.3.0)\n"); return; }

 j = byte_rchr(recip.s,recip.len,'@');
 if (j >= recip.len) { err("DSorry, address must include host name. (#5.1.3)\n"); return; }

 fdmess = open_read(messid.s);
 if (fdmess == -1) { err("Zqmail-spawn unable to open message. (#4.3.0)\n"); return; }

 if (fstat(fdmess,&st) == -1)
  { close(fdmess); err("Zqmail-spawn unable to fstat message. (#4.3.0)\n"); return; }
 if ((st.st_mode & S_IFMT) != S_IFREG)
  { close(fdmess); err("ZSorry, message has wrong type. (#4.3.5)\n"); return; }
 if (st.st_uid != auto_uidq) /* aaack! qmailq has to be trusted! */
  /* your security is already toast at this point. damage control... */
  { close(fdmess); err("ZSorry, message has wrong owner. (#4.3.5)\n"); return; }

 if (pipe(pi) == -1)
  { close(fdmess); err("Zqmail-spawn unable to create pipe. (#4.3.0)\n"); return; }

 coe(pi[0]);

 f = spawn(fdmess,pi[1],sender.s,recip.s,j);
 close(fdmess);
 if (f == -1)
  { close(pi[0]); close(pi[1]); err("Zqmail-spawn unable to fork. (#4.3.0)\n"); return; }

 d[delnum].fdin = pi[0];
 d[delnum].fdout = pi[1]; coe(pi[1]);
 d[delnum].pid = f;
 d[delnum].used = 1;
}

char cmdbuf[1024];

void getcmd()
{
 int i;
 int r;
 char ch;

 r = read(0,cmdbuf,sizeof(cmdbuf));
 if (r == 0)
  { flagreading = 0; return; }
 if (r == -1)
  {
   if (errno != error_intr)
     flagreading = 0;
   return;
  }
 
 for (i = 0;i < r;++i)
  {
   ch = cmdbuf[i];
   switch(stage)
    {
     case 0:
       delnum = (unsigned int) (unsigned char) ch;
       messid.len = 0; stage = 1; break;
     case 1:
       if (!stralloc_append(&messid,&ch)) flagabort = 1;
       if (ch) break;
       sender.len = 0; stage = 2; break;
     case 2:
       if (!stralloc_append(&sender,&ch)) flagabort = 1;
       if (ch) break;
       recip.len = 0; stage = 3; break;
     case 3:
       if (!stralloc_append(&recip,&ch)) flagabort = 1;
       if (ch) break;
       docmd();
       flagabort = 0; stage = 0; break;
    }
  }
}

char inbuf[128];

void main(argc,argv)
int argc;
char **argv;
{
 char ch;
 int i;
 int r;
 fd_set rfds;
 int nfds;

 if (chdir(auto_qmail) == -1) _exit(111);
 if (chdir("queue/mess") == -1) _exit(111);
 if (!stralloc_copys(&messid,"")) _exit(111);
 if (!stralloc_copys(&sender,"")) _exit(111);
 if (!stralloc_copys(&recip,"")) _exit(111);

 d = (struct delivery *) alloc((auto_spawn + 10) * sizeof(struct delivery));
 if (!d) _exit(111);

 substdio_fdbuf(&ssout,okwrite,1,outbuf,sizeof(outbuf));

 sig_pipeignore();
 sig_childcatch(sigchld);

 initialize(argc,argv);

 ch = auto_spawn; substdio_putflush(&ssout,&ch,1);

 for (i = 0;i < auto_spawn;++i) { d[i].used = 0; d[i].output.s = 0; }

 for (;;)
  {
   if (!flagreading)
    {
     for (i = 0;i < auto_spawn;++i) if (d[i].used) break;
     if (i >= auto_spawn) _exit(0);
    }
   sig_childunblock();

   FD_ZERO(&rfds);
   if (flagreading) FD_SET(0,&rfds);
   nfds = 1;
   for (i = 0;i < auto_spawn;++i) if (d[i].used)
    { FD_SET(d[i].fdin,&rfds); if (d[i].fdin >= nfds) nfds = d[i].fdin + 1; }

   r = select(nfds,&rfds,(fd_set *) 0,(fd_set *) 0,(struct timeval *) 0);
   sig_childblock();

   if (r != -1)
    {
     if (flagreading)
       if (FD_ISSET(0,&rfds))
	 getcmd();
     for (i = 0;i < auto_spawn;++i) if (d[i].used)
       if (FD_ISSET(d[i].fdin,&rfds))
	{
	 r = read(d[i].fdin,inbuf,128);
	 if (r == -1)
	   continue; /* read error on a readable pipe? be serious */
	 if (r == 0)
	  {
           ch = i; substdio_put(&ssout,&ch,1);
	   report(&ssout,d[i].wstat,d[i].output.s,d[i].output.len);
	   substdio_put(&ssout,"",1);
	   substdio_flush(&ssout);
	   close(d[i].fdin); d[i].used = 0;
	   continue;
	  }
	 while (!stralloc_readyplus(&d[i].output,r)) sleep(10); /*XXX*/
	 byte_copy(d[i].output.s + d[i].output.len,r,inbuf);
	 d[i].output.len += r;
	 if (truncreport > 100)
	   if (d[i].output.len > truncreport)
	    {
	     char *truncmess = "\nError report too long, sorry.\n";
	     d[i].output.len = truncreport - str_len(truncmess) - 3;
	     stralloc_cats(&d[i].output,truncmess);
	    }
	}
    }
  }
}
