#include "prot.h"
#include <sys/types.h>
#include <grp.h>
#include <unistd.h>

#ifndef GROUP_COUNT
/* use a sane default, but one can override it from outside if there are
 * higher requirements. */
#define GROUP_COUNT 8
#endif
int prot_gids(const char *user, gid_t gid)
{
  gid_t gids[GROUP_COUNT];
  int gcount = GROUP_COUNT;

  if (user) {
    int r = getgrouplist(user, gid, gids, &gcount);
    /* member of too many groups */
    if (r < 0) {
      return r;
    }
  } else {
    gcount = 1;
    gids[0] = gid;
  }

  if (setgroups(gcount,gids) == -1) return -1;
  return setgid(gid); /* _should_ be redundant, but on some systems it isn't */
}

int prot_uid(uid_t uid)
{
  return setuid(uid);
}
