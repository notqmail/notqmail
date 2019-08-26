#ifndef FMT_H
#define FMT_H

#include <stddef.h>

#define FMT_ULONG 40 /* enough space to hold 2^128 - 1 in decimal, plus \0 */
#define FMT_LEN (NULL) /* convenient abbreviation */

extern unsigned int fmt_uint(char *, unsigned int);
extern unsigned int fmt_uint0(char *, unsigned int, unsigned int);
extern unsigned int fmt_ulong(char *, unsigned long);

extern unsigned int fmt_str(char *, char *);
extern unsigned int fmt_strn(char *, char *, unsigned int);
extern unsigned int fmt_xlong(char *, unsigned long);

#endif
