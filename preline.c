#include "fd.h"
#include "sgetopt.h"
#include "readwrite.h"
#include "subfd.h"
#include "substdio.h"
#include "exit.h"
#include "fork.h"
#include "wait.h"
#include "env.h"
#include "sig.h"
#include "error.h"

void die(e,s) int e; char *s; { substdio_putsflush(subfderr,s); _exit(e); }
void die_usage() { die(100,"preline: fatal: incorrect usage\n"); }
void die_temp() { die(111,"preline: fatal: temporary problem\n"); }
void die_read() { die(111,"preline: fatal: unable to read message\n"); }
void die_badcmd() { die(100,"preline: fatal: command not found\n"); }

int flagufline = 1; char *ufline;
int flagrpline = 1; char *rpline;
int flagdtline = 1; char *dtline;

substdio ssout;
char outbuf[SUBSTDIO_OUTSIZE];
substdio ssin;
char inbuf[SUBSTDIO_INSIZE];

void main(argc,argv)
int argc;
char **argv;
{
 int opt;
 int pi[2];
 int pid;
 int wstat;

 sig_pipeignore();

 if (!(ufline = env_get("UFLINE"))) die_usage();
 if (!(rpline = env_get("RPLINE"))) die_usage();
 if (!(dtline = env_get("DTLINE"))) die_usage();

 while ((opt = getopt(argc,argv,"frdFRD")) != opteof)
   switch(opt)
    {
     case 'f': flagufline = 0; break;
     case 'r': flagrpline = 0; break;
     case 'd': flagdtline = 0; break;
     case 'F': flagufline = 1; break;
     case 'R': flagrpline = 1; break;
     case 'D': flagdtline = 1; break;
     default:
       _exit(100);
    }
 argc -= optind;
 argv += optind;
 if (!*argv) die_usage();

 if (pipe(pi) == -1) die_temp();

 switch(pid = fork())
  {
   case -1:
     die_temp();
   case 0:
     close(pi[1]);
     if (fd_move(0,pi[0])) die_temp();
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
