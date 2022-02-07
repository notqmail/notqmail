#ifndef SYNCDIR_H
#define SYNCDIR_H

int fdirsyncfn(const char *);

int syncdir_open(const char *, const int);
int syncdir_link(const char *, const char *);
int syncdir_unlink(const char *);
int syncdir_rename(const char *, const char *);

#endif
