#include <signal.h>
#include <stddef.h>

int main(void)
{
  struct sigaction sa;
  sa.sa_handler = 0;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(0,&sa,NULL);
  return 0;
}
