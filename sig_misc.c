#include <signal.h>
#include "sig.h"

void sig_miscignore()
{
  sig_catch(SIGVTALRM,SIG_IGN);
  sig_catch(SIGPROF,SIG_IGN);
  sig_catch(SIGQUIT,SIG_IGN);
  sig_catch(SIGINT,SIG_IGN);
  sig_catch(SIGHUP,SIG_IGN);
#ifdef SIGXCPU
  sig_catch(SIGXCPU,SIG_IGN);
#endif
#ifdef SIGXFSZ
  sig_catch(SIGXFSZ,SIG_IGN);
#endif
}
