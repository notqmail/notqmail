#include <signal.h>

void main(void)
{
  sigset_t ss;
 
  sigemptyset(&ss);
  sigaddset(&ss,SIGCHLD);
  sigprocmask(SIG_SETMASK,&ss,(sigset_t *) 0);
}
