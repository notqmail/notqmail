#include "timeoutread.h"
#include "select.h"
#include "error.h"
#include "readwrite.h"

int timeoutread(t,fd,buf,len) int t; int fd; char *buf; int len;
{
  fd_set rfds;
  struct timeval tv;

  tv.tv_sec = t;
  tv.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(fd,&rfds);

  if (select(fd + 1,&rfds,(fd_set *) 0,(fd_set *) 0,&tv) == -1) return -1;
  if (FD_ISSET(fd,&rfds)) return read(fd,buf,len);

  errno = error_timeout;
  return -1;
}
