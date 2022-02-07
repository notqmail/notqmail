#include "syncdir.h"

int syncdir_link(const char *oldpath, const char *newpath)
{
  if (real_link(oldpath, newpath) == -1)
    return -1;

  return fdirsyncfn(newpath);
}
