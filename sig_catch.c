#include <signal.h>
#include <stddef.h>
#include "sig.h"
#include "hassgact.h"

void sig_catch(sig,f)
int sig;
void (*f)();
{
#ifdef HASSIGACTION
  struct sigaction sa;
  sa.sa_handler = f;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(sig,&sa,NULL);
#else
  signal(sig,f); /* won't work under System V, even nowadays---dorks */
#endif
}
