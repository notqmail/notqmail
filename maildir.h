#ifndef MAILDIR_H
#define MAILDIR_H

#include "strerr.h"
extern struct strerr maildir_chdir_err;
extern struct strerr maildir_scan_err;

extern int maildir_chdir();
extern void maildir_clean();
extern int maildir_scan();

#endif
