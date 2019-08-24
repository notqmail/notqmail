#include <sys/types.h>
#include "seek.h"

int seek_trunc(int fd,seek_pos pos)
{ return ftruncate(fd,(off_t) pos); }
