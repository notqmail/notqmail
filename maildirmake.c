#include "subfd.h"
#include "substdio.h"
#include "error.h"
#include "exit.h"

void die(s) char *s; { substdio_putsflush(subfderr,s); _exit(111); }

void main(argc,argv)
int argc;
char **argv;
{
 umask(077);
 if (!argv[1]) die("usage: maildirmake name\n");
 if (mkdir(argv[1],0700))
   if (errno == error_exist) die("fatal: directory already exists\n");
   else die("fatal: unable to mkdir\n");
 if (chdir(argv[1])) die("fatal: unable to chdir\n");
 if (mkdir("tmp",0700)) die("fatal: unable to make tmp/ subdir\n");
 if (mkdir("new",0700)) die("fatal: unable to make new/ subdir\n");
 if (mkdir("cur",0700)) die("fatal: unable to make cur/ subdir\n");
 _exit(0);
}
