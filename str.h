#ifndef STR_H
#define STR_H

#include <string.h>

extern unsigned int str_copy();
extern int str_diff();
extern int str_diffn();
#define str_len(s) strlen((s))
extern unsigned int str_chr();
extern unsigned int str_rchr();
extern int str_start();

#define str_equal(s,t) (!str_diff((s),(t)))

#endif
