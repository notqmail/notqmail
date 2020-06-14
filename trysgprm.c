#include <signal.h>
#include <stddef.h>

int main(void)
{
  sigset_t ss;
 
  sigemptyset(&ss);
  sigaddset(&ss,SIGCHLD);
  sigprocmask(SIG_SETMASK,&ss,NULL);
  return 0;
}
