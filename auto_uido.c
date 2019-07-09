#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uido;

__attribute__((constructor)) static void
init_uido(void)
{
  auto_uido = inituid(auto_usero);
}
