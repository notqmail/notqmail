#include <sys/types.h>
#include <fcntl.h>
#include "open.h"
#include "syncdir.h"

int open_append(char *fn)
{
  int fd = open(fn,O_WRONLY | O_NDELAY | O_APPEND | O_CREAT,0600);
  return syncdir_open(fn,fd);
}
