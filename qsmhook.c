#include "fd.h"
#include "stralloc.h"
#include "readwrite.h"
#include "sgetopt.h"
#include "wait.h"
#include "env.h"
#include "byte.h"
#include "str.h"
#include "alloc.h"
#include "exit.h"
#include "fork.h"
#include "case.h"
#include "subfd.h"
#include "error.h"
#include "substdio.h"
#include "sig.h"

void die(e,s) int e; char *s; { substdio_putsflush(subfderr,s); _exit(e); }
void die_usage() { die(100,"qsmhook: fatal: incorrect usage\n"); }
void die_temp() { die(111,"qsmhook: fatal: temporary problem\n"); }
void die_read() { die(111,"qsmhook: fatal: unable to read message\n"); }
void die_badcmd() { die(100,"qsmhook: fatal: command not found\n"); }

int flagrpline = 0; char *rpline;
int flagufline = 1; char *ufline;
int flagdtline = 0; char *dtline;
char *host;
char *sender;
char *recip;

stralloc newarg = {0};

substdio ssout;
char outbuf[SUBSTDIO_OUTSIZE];
substdio ssin;
char inbuf[SUBSTDIO_INSIZE];

void main(argc,argv)
int argc;
char **argv;
{
 int pid;
 int wstat;
 int pi[2];
 int opt;
 char **arg;
 char *x;
 int i;
 int flagesc;

 sig_pipeignore();

 if (!(dtline = env_get("DTLINE"))) die_usage();
 if (!(rpline = env_get("RPLINE"))) die_usage();
 if (!(ufline = env_get("UFLINE"))) die_usage();
 if (!(recip = env_get("LOCAL"))) die_usage();
 if (!(host = env_get("HOST"))) die_usage();
 if (!(sender = env_get("SENDER"))) die_usage();

 while ((opt = getopt(argc,argv,"DFlMmnPsx:")) != opteof)
   switch(opt)
    {
     case 'D': case 'F': case 'M': break; /* be serious */
     case 'l': flagdtline = 1; break; /* also return-receipt-to? blech */
     case 'm': break; /* we only handle one recipient anyway */
     case 'n': flagufline = 0; break;
     case 's': break; /* could call quote() otherwise, i suppose... */
     case 'P': flagrpline = 1; break;
     case 'x':
       if (case_starts(recip,optarg))
	 recip += str_len(optarg);
       break;
     default:
       _exit(100);
    }
 argc -= optind;
 argv += optind;

 if (!*argv) die_usage();

 for (arg = argv;x = *arg;++arg)
  {
   if (!stralloc_copys(&newarg,"")) die_temp();
   flagesc = 0;
   for (i = 0;x[i];++i)
     if (flagesc)
      {
       switch(x[i])
	{
	 case '%': if (!stralloc_cats(&newarg,"%")) die_temp(); break;
	 case 'g': if (!stralloc_cats(&newarg,sender)) die_temp(); break;
	 case 'h': if (!stralloc_cats(&newarg,host)) die_temp(); break;
	 case 'u': if (!stralloc_cats(&newarg,recip)) die_temp(); break;
	}
       flagesc = 0;
      }
     else
       if (x[i] == '%')
	 flagesc = 1;
       else
	 if (!stralloc_append(&newarg,&x[i])) die_temp();
   if (!stralloc_0(&newarg)) die_temp();
   i = str_len(newarg.s) + 1;
   if (!(x = alloc(i))) die_temp();
   byte_copy(x,i,newarg.s);
   *arg = x;
  }

 if (pipe(pi) == -1) die_temp();

 switch(pid = fork())
  {
   case -1:
     die_temp();
   case 0:
     close(pi[1]);
     if (fd_move(0,pi[0]) == -1) die_temp();
     sig_pipedefault();
     execvp(*argv,argv);
     if (error_temp(errno)) die_temp();
     die_badcmd();
  }
 close(pi[0]);

 substdio_fdbuf(&ssout,write,pi[1],outbuf,sizeof(outbuf));
 substdio_fdbuf(&ssin,read,0,inbuf,sizeof(inbuf));
 if (flagufline) substdio_bputs(&ssout,ufline);
 if (flagrpline) substdio_bputs(&ssout,rpline);
 if (flagdtline) substdio_bputs(&ssout,dtline);
 if (substdio_copy(&ssout,&ssin) == -2) die_read();
 substdio_flush(&ssout);
 close(pi[1]);

 if (wait_pid(&wstat,pid) == -1) die_temp();
 if (wait_crashed(wstat)) die_temp();
 _exit(wait_exitcode(wstat));
}
