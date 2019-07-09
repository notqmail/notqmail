#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uidr;

__attribute__((constructor)) static void
uidr(void)
{
  auto_uidr = inituid(auto_userr);
}
