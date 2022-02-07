#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

int open_read(char *fn)
{
  int fd = open(fn,O_RDONLY | O_NDELAY);
  return fd;
}
