#ifndef SUBFD_H
#define SUBFD_H

#include "substdio.h"

extern substdio *subfdin;
extern substdio *subfdinsmall;
extern substdio *subfdout;
extern substdio *subfdoutsmall;
extern substdio *subfderr;

extern ssize_t subfd_read(int fd, void *buf, size_t len);
extern ssize_t subfd_readsmall(int fd, void *buf, size_t len);

#endif
