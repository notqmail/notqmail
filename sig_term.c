#include <signal.h>
#include "sig.h"

void sig_termblock() { sig_block(SIGTERM); }
void sig_termunblock() { sig_unblock(SIGTERM); }
void sig_termcatch(f) void (*f)(); { sig_catch(SIGTERM,f); }
void sig_termdefault() { sig_catch(SIGTERM,SIG_DFL); }
