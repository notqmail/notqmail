#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include "hasflock.h"
#include "lock.h"

#ifdef HASFLOCK
int lock_un(int fd) { return flock(fd,LOCK_UN); }
#else
int lock_un(int fd) { return lockf(fd,0,0); }
#endif
