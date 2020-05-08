#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include "prot.h"

int prot_gid(gid_t gid)
{
  if (setgroups(1,&gid) == -1) return -1;
  return setgid(gid); /* _should_ be redundant, but on some systems it isn't */
}

int prot_uid(uid_t uid)
{
  return setuid(uid);
}
