#ifndef TIMEOUTREAD_H
#define TIMEOUTREAD_H

#include <sys/types.h>

extern ssize_t timeoutread(int t, int fd, char *buf, size_t len);

#endif
