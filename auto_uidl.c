#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uidl;

__attribute__((constructor)) static void
init_uidl(void)
{
  auto_uidl = inituid(auto_userl);
}
