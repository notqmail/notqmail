#ifndef GEN_ALLOC_DEFS_H
#define GEN_ALLOC_DEFS_H

#define GEN_ALLOC_readyplus(ta,type,field,len,a,base,ta_rplus) \
static int ta_rplus ## _internal (ta *x, unsigned int n, unsigned int pluslen) \
{ \
  if (x->field) { \
    unsigned int nnum; \
    n += pluslen; \
    if (n <= x->a) \
      return 1; \
    nnum = base + n + (n >> 3); \
    if (!alloc_re(&x->field,x->a * sizeof(type),nnum * sizeof(type))) \
      return 0; \
    x->a = nnum; \
    return 1; } \
  x->len = 0; \
  return !!(x->field = (type *) alloc((x->a = n) * sizeof(type))); } \
int ta_rplus(ta *x, unsigned int n) \
{ return ta_rplus ## _internal (x, n, x->len); }

/* this needs a GEN_ALLOC_readyplus call before as it reuses the internal helper
 * function. */
#define GEN_ALLOC_ready(ta,type,field,len,a,base,ta_ready) \
int ta_ready(ta *x, unsigned int n) \
{ return ta_ready ## plus_internal (x, n, 0); }

#define GEN_ALLOC_append(ta,type,field,len,a,base,ta_rplus,ta_append) \
int ta_append(ta *x, type *i) \
{ if (!ta_rplus(x,1)) return 0; x->field[x->len++] = *i; return 1; }

#endif
