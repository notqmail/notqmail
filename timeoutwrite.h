#ifndef TIMEOUTWRITE_H
#define TIMEOUTWRITE_H

#define TIMEOUTWRITE(s,fd) (((s) << 10) | (fd))

extern int timeoutwrite();

#endif
