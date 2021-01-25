#include "coe.h"

#include <fcntl.h>

int coe(fd)
int fd;
{
  return fcntl(fd,F_SETFD,1);
}
