#include "fd.h"
#include "wait.h"
#include "substdio.h"
#include "exit.h"
#include "fork.h"
#include "error.h"
#include "tcpto.h"

void initialize(argc,argv)
int argc;
char **argv;
{
 tcpto_clean();
}

int truncreport = 0;

void report(ss,wstat,s,len)
substdio *ss;
int wstat;
char *s;
int len;
{
 int j;
 int k;
 int result;
 int orr;

 if (wait_crashed(wstat))
  { substdio_puts(ss,"Zqmail-remote crashed.\n"); return; }
 switch(wait_exitcode(wstat))
  {
   case 0: break;
   case 111: substdio_puts(ss,"ZUnable to run qmail-remote.\n"); return;
   default: substdio_puts(ss,"DUnable to run qmail-remote.\n"); return;
  }
 if (!len)
  { substdio_puts(ss,"Zqmail-remote produced no output.\n"); return; }

 result = -1;
 j = 0;
 for (k = 0;k < len;++k)
   if (!s[k])
    {
     if (s[j] == 'K') { result = 1; break; }
     if (s[j] == 'Z') { result = 0; break; }
     if (s[j] == 'D') break;
     j = k + 1;
    }

 orr = result;
 switch(s[0])
  {
   case 's': orr = 0; break;
   case 'h': orr = -1;
  }

 switch(orr)
  {
   case 1: substdio_put(ss,"K",1); break;
   case 0: substdio_put(ss,"Z",1); break;
   case -1: substdio_put(ss,"D",1); break;
  }

 for (k = 1;k < len;)
   if (!s[k++])
    {
     substdio_puts(ss,s + 1);
     if (result <= orr)
       if (k < len)
	 switch(s[k])
	  {
	   case 'Z': case 'D': case 'K':
             substdio_puts(ss,s + k + 1);
	  }
     break;
    }
}

int spawn(fdmess,fdout,s,r,at)
int fdmess; int fdout;
char *s; char *r; int at;
{
 int f;
 char *(args[5]);

 args[0] = "qmail-remote";
 args[1] = r + at + 1;
 args[2] = s;
 args[3] = r;
 args[4] = 0;

 if (!(f = vfork()))
  {
   if (fd_move(0,fdmess) == -1) _exit(111);
   if (fd_move(1,fdout) == -1) _exit(111);
   if (fd_copy(2,1) == -1) _exit(111);
   execvp(*args,args);
   if (error_temp(errno)) _exit(111);
   _exit(100);
  }
 return f;
}
