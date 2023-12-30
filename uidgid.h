#ifndef UIDGID_H
#define UIDGID_H

#include <sys/types.h>

extern uid_t inituid(char *uid);
extern gid_t initgid(char *gid);

#endif
