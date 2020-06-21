#ifndef OPEN_H
#define OPEN_H

extern int open_read(const char *fn);
extern int open_excl(const char *fn);
extern int open_append(const char *fn);
extern int open_trunc(const char *fn);
extern int open_write(const char *fn);

#endif
