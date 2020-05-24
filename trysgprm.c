#include <signal.h>

int main(void)
{
  sigset_t ss;
 
  sigemptyset(&ss);
  sigaddset(&ss,SIGCHLD);
  sigprocmask(SIG_SETMASK,&ss,(sigset_t *) 0);
  return 0;
}
