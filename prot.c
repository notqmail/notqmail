#include "prot.h"

#include <unistd.h>

/* XXX: there are more portability problems here waiting to leap out at me */

int prot_gid(gid_t gid)
{
  if (setgroups(1,&gid) == -1) return -1;
  return setgid(gid); /* _should_ be redundant, but on some systems it isn't */
}

int prot_uid(uid_t uid)
{
  return setuid(uid);
}
