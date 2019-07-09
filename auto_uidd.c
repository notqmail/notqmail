#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uidd;

__attribute__((constructor)) static void
init_uidd(void)
{
  auto_uidd = inituid(auto_userd);
}
