#include "alloc.h"
#include "gen_allocdefs.h"
#include "stralloc.h"
#include "strsalloc.h"

GEN_ALLOC_readyplus(strsalloc,stralloc,sa,len,a,i,n,x,10,strsalloc_readyplus)
GEN_ALLOC_append(strsalloc,stralloc,sa,len,a,i,n,x,10,strsalloc_readyplus,strsalloc_append)
