#include "timeoutread.h"
#include "select.h"
#include "error.h"
#include "readwrite.h"

int timeoutread(fdt,buf,len) int fdt; char *buf; int len;
{
  fd_set rfds;
  struct timeval tv;
  int fd;

  tv.tv_sec = (fdt >> 10);
  tv.tv_usec = 0;

  fd = (fdt & 1023);
  FD_ZERO(&rfds);
  FD_SET(fd,&rfds);

  if (select(fd + 1,&rfds,(fd_set *) 0,(fd_set *) 0,&tv) == -1) return -1;
  if (FD_ISSET(fd,&rfds)) return read(fd,buf,len);

  shutdown(fd,0);
  errno = error_timeout;
  return -1;
}
