#ifndef PROT_H
#define PROT_H

#include <sys/types.h>

extern int prot_gid(gid_t gid);
extern int prot_gids(const char *user, gid_t gid);
extern int prot_uid(uid_t uid);

#endif
