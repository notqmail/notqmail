#include <sys/types.h>
#include <sys/wait.h>
#include "error.h"

/* restriction: you must not care about any other child. */
int wait_pid(wstat,pid) int *wstat; int pid;
{
  int r;
  do
    r = wait(wstat);
  while ((r != pid) && ((r != -1) || (errno == error_intr)));
  return r;
}
