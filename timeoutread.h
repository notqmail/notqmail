#ifndef TIMEOUTREAD_H
#define TIMEOUTREAD_H

#define TIMEOUTREAD(s,fd) (((s) << 10) | (fd))

extern int timeoutread();

#endif
