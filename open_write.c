#include <sys/types.h>
#include <fcntl.h>
#include "fsyncdir.h"
#include "open.h"

int open_write(char *fn)
{
  int fd = open(fn,O_WRONLY | O_NDELAY);
  return schmonz(fn,fd);
}
