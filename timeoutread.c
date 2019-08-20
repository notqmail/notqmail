#include "timeoutread.h"
#include "select.h"
#include "error.h"
#include "readwrite.h"

ssize_t timeoutread(int t, int fd, char *buf, size_t len)
{
  fd_set rfds;
  struct timeval tv;

  tv.tv_sec = t;
  tv.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(fd,&rfds);

  if (select(fd + 1,&rfds,NULL,NULL,&tv) == -1) return -1;
  if (FD_ISSET(fd,&rfds)) return read(fd,buf,len);

  errno = error_timeout;
  return -1;
}
