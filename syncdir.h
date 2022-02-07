#ifndef SYNCDIR_H
#define SYNCDIR_H

/* internal */
int fdirsyncfn(const char *);

/* call these wrappers instead of the syscalls directly */
int syncdir_open(const char *, const int);
int syncdir_link(const char *, const char *);
int syncdir_unlink(const char *);
int syncdir_rename(const char *, const char *);

#endif
