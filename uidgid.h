#ifndef UIDGID_H
#define UIDGID_H

#include <sys/types.h>

extern uid_t inituid(const char *uid);
extern gid_t initgid(const char *gid);

#endif
