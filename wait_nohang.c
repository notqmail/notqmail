#include <sys/types.h>
#include <sys/wait.h>
#include <stddef.h>
#include "haswaitp.h"

int wait_nohang(wstat) int *wstat;
{
#ifdef HASWAITPID
  return waitpid(-1,wstat,WNOHANG);
#else
  return wait3(wstat,WNOHANG,NULL);
#endif
}
