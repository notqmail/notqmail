#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include "hasflock.h"
#include "lock.h"

#ifdef HASFLOCK
int lock_ex(fd) int fd; { return flock(fd,LOCK_EX); }
#else
int lock_ex(fd) int fd; { return lockf(fd,1,0); }
#endif
