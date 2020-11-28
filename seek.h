#ifndef SEEK_H
#define SEEK_H

#include <sys/types.h>
#include <unistd.h>

typedef off_t seek_pos;

#define seek_cur(fd) (lseek((fd), 0, SEEK_CUR))

static inline int seek_set(int fd, seek_pos pos)
{
  if (lseek(fd, pos, SEEK_SET) == -1)
    return -1;
  return 0;
}


static inline int seek_end(int fd)
{
  if (lseek(fd, 0, SEEK_END) == -1)
    return -1;
  return 0;
}

#define seek_trunc(fd, pos) (ftruncate((fd),(pos)))
#define seek_begin(fd) (seek_set((fd),(seek_pos) 0))

#endif
