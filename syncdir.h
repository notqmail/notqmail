#ifndef SYNCDIR_H
#define SYNCDIR_H

/* internal */
int fdirsyncfn(const char *);

/* XXX better name; this is called from open_*() */
int syncdir_open(const char *, const int);

/* call these wrappers instead of the syscalls directly */
int syncdir_link(const char *, const char *);
int syncdir_unlink(const char *);
int syncdir_rename(const char *, const char *);

int real_link(const char *, const char *);
int real_rename(const char *, const char *);
int real_unlink(const char *);

#define link syncdir_link
#define rename syncdir_rename
#define unlink syncdir_unlink

#endif
