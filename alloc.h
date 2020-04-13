#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

#define alloc malloc
#define alloc_free(x) free(x)
#define alloc_re(x,m,n) realloc(x,n)

#endif
