#include <fcntl.h>
#include "coe.h"

int coe(fd)
int fd;
{
  return fcntl(fd,F_SETFD,1);
}
