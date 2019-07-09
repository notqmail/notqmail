#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uidq;

__attribute__((constructor)) static void
uidq(void)
{
  auto_uidq = inituid(auto_userq);
}
