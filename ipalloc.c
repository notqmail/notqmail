#include "gen_allocdefs.h"
#include "ip.h"
#include "ipalloc.h"

GEN_ALLOC_readyplus(ipalloc,struct ip_mx,ix,len,a,10,ipalloc_readyplus)
GEN_ALLOC_append(ipalloc,struct ip_mx,ix,len,a,10,ipalloc_readyplus,ipalloc_append)
