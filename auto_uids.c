#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uids;

__attribute__((constructor)) static void
uids(void)
{
  auto_uids = inituid(auto_users);
}
