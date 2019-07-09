#include <sys/types.h>
#include <grp.h>
#include "uidgid.h"
#include "subfd.h"
#include "substdio.h"
#include "exit.h"

int
initgid(group)
  char *group;
{
  struct group *gr;
  gr = getgrnam(group);
  if (!gr) {
    substdio_puts(subfderr,"fatal: unable to find group ");
    substdio_puts(subfderr,group);
    substdio_puts(subfderr,"\n");
    substdio_flush(subfderr);
    _exit(111);
  }
  return gr->gr_gid;
}
