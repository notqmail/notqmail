#ifndef PROT_H
#define PROT_H

#include <sys/types.h>

#define prot_gid(gid) prot_gids(NULL,(gid))
extern int prot_gids(const char *user, gid_t gid);
extern int prot_uid(uid_t uid);

#endif
