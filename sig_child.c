#include <signal.h>
#include "sig.h"

void sig_childblock() { sig_block(SIGCHLD); }
void sig_childunblock() { sig_unblock(SIGCHLD); }
void sig_childcatch(f) void (*f)(); { sig_catch(SIGCHLD,f); }
void sig_childdefault() { sig_catch(SIGCHLD,SIG_DFL); }
