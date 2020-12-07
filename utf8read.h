#ifndef UTF8READ_H
#define UTF8READ_H
#include "stralloc.h"

extern int utf8read();
extern int containsutf8(unsigned char *, int);
extern stralloc header;
extern int flagutf8;

#endif
