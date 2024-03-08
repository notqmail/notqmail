#ifndef STR_H
#define STR_H

extern unsigned int str_copy();
extern unsigned int str_copyb();
extern int str_diff();
extern int str_diffn();
extern unsigned int str_len();
extern unsigned int str_chr();
extern unsigned int str_rchr();
extern int str_start();
#include <sys/types.h>
extern size_t str_cspn();

#define str_equal(s,t) (!str_diff((s),(t)))

#endif
