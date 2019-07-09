#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_gidq;

__attribute__((constructor)) static void
init_gidq(void)
{
  auto_gidq = inituid(auto_groupq);
}
