#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "uidgid.h"
#include "subfd.h"
#include "substdio.h"

uid_t
inituid(char *user)
{
  struct passwd *pw;
  pw = getpwnam(user);
  if (!pw) {
    substdio_puts(subfderr,"fatal: unable to find user ");
    substdio_puts(subfderr,user);
    substdio_puts(subfderr,"\n");
    substdio_flush(subfderr);
    _exit(111);
  }
  return pw->pw_uid;
}
