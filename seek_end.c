#include <sys/types.h>
#include "seek.h"

#define END 2 /* sigh */

int seek_end(fd) int fd;
{ if (lseek(fd,(off_t) 0,END) == -1) return -1; return 0; }
