#include <signal.h>
#include "sig.h"

void sig_bugcatch(f) void (*f)();
{
  sig_catch(SIGILL,f);
  sig_catch(SIGABRT,f);
  sig_catch(SIGFPE,f);
  sig_catch(SIGBUS,f);
  sig_catch(SIGSEGV,f);
#ifdef SIGSYS
  sig_catch(SIGSYS,f);
#endif
#ifdef SIGEMT
  sig_catch(SIGEMT,f);
#endif
}
