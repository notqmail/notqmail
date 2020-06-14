#include <signal.h>
#include <stddef.h>
#include "sig.h"
#include "hassgprm.h"

void sig_block(sig)
int sig;
{
#ifdef HASSIGPROCMASK
  sigset_t ss;
  sigemptyset(&ss);
  sigaddset(&ss,sig);
  sigprocmask(SIG_BLOCK,&ss,NULL);
#else
  sigblock(1 << (sig - 1));
#endif
}

void sig_unblock(sig)
int sig;
{
#ifdef HASSIGPROCMASK
  sigset_t ss;
  sigemptyset(&ss);
  sigaddset(&ss,sig);
  sigprocmask(SIG_UNBLOCK,&ss,NULL);
#else
  sigsetmask(sigsetmask(~0) & ~(1 << (sig - 1)));
#endif
}

void sig_blocknone()
{
#ifdef HASSIGPROCMASK
  sigset_t ss;
  sigemptyset(&ss);
  sigprocmask(SIG_SETMASK,&ss,NULL);
#else
  sigsetmask(0);
#endif
}
