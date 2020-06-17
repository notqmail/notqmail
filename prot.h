#ifndef PROT_H
#define PROT_H

#include <sys/types.h>
#include <unistd.h>

#define prot_gid(gid) prot_gids(NULL,(gid))
extern int prot_gids(const char *user, gid_t gid);
#define prot_uid(uid) setuid(uid)

#endif
