#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_gidn;

__attribute__((constructor)) static void
init_gidn(void)
{
  auto_gidn = inituid(auto_groupn);
}
