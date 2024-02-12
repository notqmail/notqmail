#ifndef STR_H
#define STR_H

#include <string.h>

#define str_copy(s,t) strcpy((s),(t))
extern unsigned int str_copyb();
#define str_diff(s,t) strcmp((s),(t))
#define str_diffn(s,t,len) strncmp((s),(t),(len))
#define str_len(s) strlen((s))
extern unsigned int str_chr();
extern unsigned int str_rchr();
extern int str_start();
#include <sys/types.h>
extern size_t str_cspn();

#define str_equal(s,t) (strcmp((s),(t)) == 0)

#endif
