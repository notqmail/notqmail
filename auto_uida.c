#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uida;

__attribute__((constructor)) static void
init_uida(void)
{
  auto_uida = inituid(auto_usera);
}
