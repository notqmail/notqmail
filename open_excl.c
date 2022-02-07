#include <sys/types.h>
#include <fcntl.h>
#include "fsyncdir.h"
#include "open.h"

int open_excl(char *fn)
{
  int fd = open(fn,O_WRONLY | O_EXCL | O_CREAT,0644);
  return schmonz(fn,fd);
}
