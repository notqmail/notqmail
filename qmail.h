#ifndef QMAIL_H
#define QMAIL_H

#include "substdio.h"

struct qmail {
  int flagerr;
  unsigned long pid;
  int fdm;
  int fde;
  substdio ss;
  char buf[1024];
} ;

extern int qmail_open();
extern void qmail_put();
extern void qmail_puts();
extern void qmail_from();
extern void qmail_to();
extern void qmail_fail();
extern int qmail_close();
extern unsigned long qmail_qp();

#define QMAIL_WAITPID -2
#define QMAIL_CRASHED -3
#define QMAIL_USAGE -4
#define QMAIL_BUG -5
#define QMAIL_SYS -6
#define QMAIL_READ -7
#define QMAIL_WRITE -8
#define QMAIL_NOMEM -9
#define QMAIL_EXECSOFT -11
#define QMAIL_TIMEOUT -13
#define QMAIL_TOOLONG -14

#endif
