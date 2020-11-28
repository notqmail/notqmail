#ifndef HIER_H
#define HIER_H

#include <sys/types.h>

extern void h(char *home, uid_t uid, gid_t gid, int mode);
extern void d(char *home, char *subdir, uid_t uid, gid_t gid, int mode);
extern void p(char *home, char *fifo, uid_t uid, gid_t gid, int mode);
extern void c(char *home, char *subdir, char *file, uid_t uid, gid_t gid, int mode);
extern void z(char *home, char *file, int len, uid_t uid, gid_t gid, int mode);
extern void hier(void);

#endif
