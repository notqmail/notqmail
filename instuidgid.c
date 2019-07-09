#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

int auto_uida;
int auto_uido;
int auto_uidq;
int auto_uidr;
int auto_uids;

int auto_gidq;

void
init_uidgid()
{
  auto_uida = inituid(auto_usera);
  auto_uido = inituid(auto_usero);
  auto_uidq = inituid(auto_userq);
  auto_uidr = inituid(auto_userr);
  auto_uids = inituid(auto_users);

  auto_gidq = initgid(auto_groupq);
}
