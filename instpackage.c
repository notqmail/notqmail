#include "hier.h"

#include <sys/stat.h>
#include <stddef.h>
#include "open.h"
#include "strerr.h"

#define FATAL "instpackage: fatal: "

extern int fdsourcedir;

int main(void)
{
  fdsourcedir = open_read(".");
  if (fdsourcedir == -1)
    strerr_die2sys(111,FATAL,"unable to open current directory: ");

  umask(077);
  hier();
  return 0;
}
