#ifndef TIMEOUTWRITE_H
#define TIMEOUTWRITE_H

#include <sys/types.h>

extern ssize_t timeoutwrite(int t, int fd, const void *buf, size_t len);

#define GEN_SAFE_TIMEOUTWRITE(funcname,tout,writefd,doexit) \
ssize_t funcname(int fd, const void *buf, size_t len) \
{ \
  ssize_t r; \
  r = timeoutwrite(tout,writefd,buf,len); \
  if (r == 0 || r == -1) doexit; \
  return r; \
}

#endif
