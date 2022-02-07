#ifndef SYNCDIR_H
#define SYNCDIR_H

int fsync_after_open_or_bust(const char *, const int);
int syncdir_link(const char *, const char *);
int syncdir_unlink(const char *);
int syncdir_rename(const char *, const char *);

#endif
