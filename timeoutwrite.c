#include "timeoutwrite.h"
#include "select.h"
#include "error.h"
#include "readwrite.h"

int timeoutwrite(fdt,buf,len) int fdt; char *buf; int len;
{
  fd_set wfds;
  struct timeval tv;
  int fd;

  tv.tv_sec = (fdt >> 10);
  tv.tv_usec = 0;

  fd = (fdt & 1023);
  FD_ZERO(&wfds);
  FD_SET(fd,&wfds);

  if (select(fd + 1,(fd_set *) 0,&wfds,(fd_set *) 0,&tv) == -1) return -1;
  if (FD_ISSET(fd,&wfds)) return write(fd,buf,len);

  shutdown(fd,1);
  errno = error_timeout;
  return -1;
}
