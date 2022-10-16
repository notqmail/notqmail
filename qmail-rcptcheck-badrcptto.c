#include <unistd.h>
#include "badrcptto.h"
#include "env.h"

void accept_recipient() { _exit(  0); }
void reject_recipient() { _exit(100); }
void unable_to_verify() { _exit(111); }

void die_control() { unable_to_verify(); }
void die_nomem()   { unable_to_verify(); }

int main(void)
{
  char *recipient = env_get("RECIPIENT");

  if (!recipient)
    unable_to_verify();

  if (badrcptto_reject_recipient(recipient))
    reject_recipient();

  accept_recipient();
}
