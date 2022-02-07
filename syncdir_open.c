#include <unistd.h>
#include "syncdir.h"

int syncdir_open(const char *fn, const int fd)
{
  if (fd == -1)
    return fd;

  if (fdirsyncfn(fn) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}
