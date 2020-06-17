#ifndef PROT_H
#define PROT_H

#include <sys/types.h>
#include <unistd.h>

extern int prot_gid(gid_t gid);
extern int prot_gids(const char *user, gid_t gid);
#define prot_uid(uid) setuid(uid)

#endif
