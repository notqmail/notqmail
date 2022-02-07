#include <sys/types.h>
#include <fcntl.h>
#include "open.h"
#include "syncdir.h"

int open_excl(char *fn)
{
  int fd = open(fn,O_WRONLY | O_EXCL | O_CREAT,0644);
  return syncdir_open(fn,fd);
}
