#ifndef BYTE_H
#define BYTE_H

extern unsigned int byte_chr();
extern unsigned int byte_rchr();
extern unsigned int byte_cspn();
extern unsigned int byte_rcspn();
extern void byte_copy();
extern void byte_copyr();
extern int byte_diff();
extern void byte_zero();

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif
