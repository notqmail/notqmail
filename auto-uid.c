#include <sys/types.h>
#include <pwd.h>
#include "subfd.h"
#include "substdio.h"
#include "readwrite.h"
#include "exit.h"
#include "scan.h"
#include "fmt.h"

char buf1[256];
substdio ss1 = SUBSTDIO_FDBUF(write,1,buf1,sizeof(buf1));

void outs(s) /* was named puts, but Solaris pwd.h includes stdio.h. dorks. */
char *s;
{
  if (substdio_puts(&ss1,s) == -1) _exit(111);
}

void main(argc,argv)
int argc;
char **argv;
{
  char *name;
  char *value;
  struct passwd *pw;
  char strnum[FMT_ULONG];

  name = argv[1];
  if (!name) _exit(100);
  value = argv[2];
  if (!value) _exit(100);

  pw = getpwnam(value);
  if (!pw) {
    substdio_puts(subfderr,"fatal: unable to find user ");
    substdio_puts(subfderr,value);
    substdio_puts(subfderr,"\n");
    substdio_flush(subfderr);
    _exit(111);
  }

  strnum[fmt_ulong(strnum,(unsigned long) pw->pw_uid)] = 0;

  outs("int ");
  outs(name);
  outs(" = ");
  outs(strnum);
  outs(";\n");
  if (substdio_flush(&ss1) == -1) _exit(111);
  _exit(0);
}
