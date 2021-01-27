#include "prot.h"

#include <sys/types.h>
#include <grp.h>
#include <unistd.h>

int prot_gid(gid_t gid)
{
  if (setgroups(1,&gid) == -1) return -1;

  return setgid(gid); /* _should_ be redundant, but on some systems it isn't */
}

int prot_gids(const char *user, gid_t gid)
{
  /* member of too many groups */
  if (initgroups(user, gid) == -1) return -1;

  return setgid(gid); /* _should_ be redundant, but on some systems it isn't */
}
