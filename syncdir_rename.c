#include "syncdir.h"

int syncdir_rename(const char *oldpath, const char *newpath)
{
  if (real_rename(oldpath,newpath) == -1)
    return -1;

  if (fdirsyncfn(newpath) == -1)
    return -1;

  return fdirsyncfn(oldpath);
}
