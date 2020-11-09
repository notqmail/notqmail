#ifndef HIER_H
#define HIER_H

#include <sys/types.h>

extern void h(const char *home, uid_t uid, gid_t gid, int mode);
extern void d(const char *home, const char *subdir, uid_t uid, gid_t gid, int mode);
extern void p(const char *home, const char *fifo, uid_t uid, gid_t gid, int mode);
extern void c(const char *home, const char *subdir, const char *file, uid_t uid, gid_t gid, int mode);
extern void z(const char *home, const char *file, int len, uid_t uid, gid_t gid, int mode);
extern void hier(void);
extern void hier_queue(void);

#endif
