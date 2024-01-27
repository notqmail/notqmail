#include "hier.h"

#include <sys/stat.h>
#include <stddef.h>
#include <string.h>
#include "open.h"
#include "strerr.h"

#define FATAL "instpackage: fatal: "

extern int fdsourcedir;

int main(int argc, char **argv)
{
  fdsourcedir = open_read(".");
  if (fdsourcedir == -1)
    strerr_die2sys(111,FATAL,"unable to open current directory: ");

  umask(077);
  if (argc == 2 && strcmp(argv[1],"queue-only") == 0)
    hier_queue();
  else
    hier();
  return 0;
}
