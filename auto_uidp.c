#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uidp;

__attribute__((constructor)) static void
init_uidp(void)
{
  auto_uidp = inituid(auto_userp);
}
