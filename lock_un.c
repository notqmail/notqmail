#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include "hasflock.h"
#include "lock.h"

#ifdef HASFLOCK
int lock_un(fd) int fd; { return flock(fd,LOCK_UN); }
#else
int lock_un(fd) int fd; { return lockf(fd,0,0); }
#endif
