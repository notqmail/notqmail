#ifndef TIMEOUTREAD_H
#define TIMEOUTREAD_H

#include <sys/types.h>

extern ssize_t timeoutread(int t, int fd, char *buf, size_t len);

#define GEN_SAFE_TIMEOUTREAD(funcname,tout,readfd,doexit) \
ssize_t funcname(int fd, void *buf, size_t len) \
{ \
  ssize_t r; \
  r = timeoutread(tout,readfd,buf,len); \
  if (r == 0 || r == -1) doexit; \
  return r; \
}

#endif
