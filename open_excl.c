#include <sys/types.h>
#include <fcntl.h>
#include "syncdir.h"
#include "open.h"

int open_excl(char *fn)
{
  int fd = open(fn,O_WRONLY | O_EXCL | O_CREAT,0644);
  return fsync_after_open_or_bust(fn,fd);
}
