#ifndef STR_H
#define STR_H

#include <string.h>

extern unsigned int str_copy();
#define str_diff(s,t) strcmp((s),(t))
extern int str_diffn();
#define str_len(s) strlen((s))
extern unsigned int str_chr();
extern unsigned int str_rchr();
extern int str_start();

#define str_equal(s,t) (strcmp((s),(t)) == 0)

#endif
