#include <signal.h>
#include "sig.h"

void sig_pipeignore() { sig_catch(SIGPIPE,SIG_IGN); }
void sig_pipedefault() { sig_catch(SIGPIPE,SIG_DFL); }
