#ifndef BYTE_H
#define BYTE_H

#include <string.h>

extern unsigned int byte_chr(char *s, unsigned int n, int c);
extern unsigned int byte_rchr(char *s, unsigned int n, int c);
extern void byte_copy(char *to, unsigned int n, char *from);
extern void byte_copyr(char *to, unsigned int n, char *from);
extern void byte_zero(char *s, unsigned int n);

#define byte_equal(s,n,t) (memcmp((s),(t),(n)) == 0)

#endif
