#include "syncdir.h"

int syncdir_unlink(const char *path)
{
  if (real_unlink(path) == -1)
    return -1;

  return fdirsyncfn(path);
}
