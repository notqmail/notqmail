#include <sys/types.h>
#include <fcntl.h>
#include "syncdir.h"
#include "open.h"

int open_write(char *fn)
{
  int fd = open(fn,O_WRONLY | O_NDELAY);
  return fsync_after_open_or_bust(fn,fd);
}
