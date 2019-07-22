#include <sys/types.h>
#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

uid_t auto_uida;
uid_t auto_uido;
uid_t auto_uidq;
uid_t auto_uidr;
uid_t auto_uids;

gid_t auto_gidq;

void
init_uidgid()
{
  auto_uida = inituid(auto_usera);
  auto_uido = inituid(auto_usero);
  auto_uidq = inituid(auto_userq);
  auto_uidr = inituid(auto_userr);
  auto_uids = inituid(auto_users);

  auto_gidq = initgid(auto_groupq);
}
