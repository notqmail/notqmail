#include <sys/types.h>
#include <fcntl.h>
#include "syncdir.h"
#include "open.h"

int open_append(char *fn)
{
  int fd = open(fn,O_WRONLY | O_NDELAY | O_APPEND | O_CREAT,0600);
  return fsync_after_open_or_bust(fn,fd);
}
