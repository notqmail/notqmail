#include "stralloc.h"
#include "gen_allocdefs.h"

GEN_ALLOC_append(stralloc,const char,s,len,a,30,stralloc_readyplus,stralloc_append)
