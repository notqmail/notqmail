#include "timeoutwrite.h"
#include "select.h"
#include "error.h"
#include "readwrite.h"

int timeoutwrite(t,fd,buf,len) int t; int fd; char *buf; int len;
{
  fd_set wfds;
  struct timeval tv;

  tv.tv_sec = t;
  tv.tv_usec = 0;

  FD_ZERO(&wfds);
  FD_SET(fd,&wfds);

  if (select(fd + 1,(fd_set *) 0,&wfds,(fd_set *) 0,&tv) == -1) return -1;
  if (FD_ISSET(fd,&wfds)) return write(fd,buf,len);

  errno = error_timeout;
  return -1;
}
