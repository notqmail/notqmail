#ifndef QUOTE_H
#define QUOTE_H

#include "stralloc.h"

extern int quote_need(char*,unsigned int);
extern int quote(stralloc*,stralloc*);
extern int quote2(stralloc*,char*);

#endif
