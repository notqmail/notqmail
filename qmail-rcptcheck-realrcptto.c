#include <unistd.h>
#include "env.h"
#include "realrcptto.h"

void accept_recipient() { _exit(  0); }
void reject_recipient() { _exit(100); }
void unable_to_verify() { _exit(111); }

void die_cdb()     { unable_to_verify(); }
void die_control() { unable_to_verify(); }
void die_nomem()   { unable_to_verify(); }
void die_sys()     { unable_to_verify(); }

int main(void)
{
  char *recipient = env_get("RECIPIENT");

  if (!recipient)
    unable_to_verify();

  realrcptto_init();
  realrcptto_start();
  if (!realrcptto(recipient))
    reject_recipient();

  accept_recipient();
}
