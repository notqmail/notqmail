#include "sgetopt.h"
#include "substdio.h"
#include "subfd.h"
#include "alloc.h"
#include "auto_qmail.h"
#include "exit.h"
#include "env.h"
#include "str.h"

void nomem()
{
 substdio_putsflush(subfderr,"sendmail: fatal: out of memory\n");
 _exit(111);
}

int flagh;
char *sender;

void main(argc,argv)
int argc;
char **argv;
{
 int opt;
 char **qiargv;
 char **arg;
 int i;

 if (chdir(auto_qmail) == -1)
  {
   substdio_putsflush(subfderr,"sendmail: fatal: unable to switch to qmail home directory\n");
   _exit(111);
  }

 flagh = 0;
 sender = 0;
 while ((opt = getopt(argc,argv,"vimte:f:p:o:B:F:EJx")) != opteof)
   switch(opt)
    {
     case 'B': break;
     case 't': flagh = 1; break;
     case 'f': sender = optarg; break;
     case 'F': if (!env_put2("MAILNAME",optarg)) nomem(); break;
     case 'p': break; /* could generate a Received line from optarg */
     case 'v': break;
     case 'i': break; /* what an absurd concept */
     case 'x': break; /* SVR4 stupidity */
     case 'm': break; /* twisted-paper-path blindness, incompetent design */
     case 'e': break; /* qmail has only one error mode */
     case 'o':
       switch(optarg[0])
	{
	 case 'd': break; /* qmail has only one delivery mode */
	 case 'e': break; /* see 'e' above */
	 case 'i': break; /* see 'i' above */
	 case 'm': break; /* see 'm' above */
	}
       break;
     case 'E': case 'J': /* Sony NEWS-OS */
       while (argv[optind][optpos]) ++optpos; /* skip optional argument */
       break;
     default:
       _exit(100);
    }
 argc -= optind;
 argv += optind;

 if (str_equal(optprogname,"mailq"))
  {
   substdio_putsflush(subfderr,"sendmail: fatal: please use qmail-qread instead\n");
   _exit(100);
  }

 if (str_equal(optprogname,"newaliases"))
  {
   substdio_putsflush(subfderr,"sendmail: fatal: please use the qmsmac newaliases instead\n");
   _exit(100);
  }

 qiargv = (char **) alloc((argc + 10) * sizeof(char *));
 if (!qiargv) nomem();

 arg = qiargv;
 *arg++ = "bin/qmail-inject";
 *arg++ = (flagh ? "-H" : "-a");
 if (sender)
  {
   *arg++ = "-f";
   *arg++ = sender;
  }
 *arg++ = "--";
 for (i = 0;i < argc;++i) *arg++ = argv[i];
 *arg = 0;

 execv(*qiargv,qiargv);
 
 substdio_putsflush(subfderr,"sendmail: fatal: unable to run qmail-inject\n");
 _exit(111);
}
