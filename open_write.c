#include <sys/types.h>
#include <fcntl.h>
#include "syncdir.h"
#include "open.h"

int open_write(char *fn)
{
  int fd = open(fn,O_WRONLY | O_NDELAY);
  return syncdir_open(fn,fd);
}
