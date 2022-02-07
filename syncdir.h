#ifndef FDIRSYNC_H
#define FDIRSYNC_H

int fsync_after_open_or_bust(const char *, const int);
int fdirsyncfn(const char *);
int linksync(const char *, const char *);
int unlinksync(const char *);

#endif
