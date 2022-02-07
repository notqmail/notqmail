#include <unistd.h>
#include "fsyncdir.h"

int linksync(const char *oldpath, const char *newpath)
{
  if (link(oldpath, newpath) == -1)
    return -1;

  return fdirsyncfn(newpath);
}
