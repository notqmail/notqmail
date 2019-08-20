#ifndef TIMEOUTWRITE_H
#define TIMEOUTWRITE_H

#include <sys/types.h>

extern ssize_t timeoutwrite(int t, int fd, const void *buf, size_t len);

#endif
