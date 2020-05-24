#include <signal.h>

int main(void)
{
  struct sigaction sa;
  sa.sa_handler = 0;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(0,&sa,(struct sigaction *) 0);
  return 0;
}
