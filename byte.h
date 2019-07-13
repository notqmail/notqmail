#ifndef BYTE_H
#define BYTE_H

#include <string.h>

extern unsigned int byte_chr();
extern unsigned int byte_rchr();
extern unsigned int byte_cspn();
extern unsigned int byte_rcspn();
extern void byte_copy();
extern void byte_copyr();
extern void byte_zero();

#define byte_equal(s,n,t) (memcmp((s),(t),(n)) == 0)

#endif
