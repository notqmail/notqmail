/*
 * $Log: base64.h,v $
 * Revision 1.3  2004-06-18 22:55:43+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef BASE64_H
#define BASE64_H
#include "stralloc.h"

extern int b64decode(const unsigned char *, int, stralloc *);
extern int b64encode(stralloc *, stralloc *);

#endif
