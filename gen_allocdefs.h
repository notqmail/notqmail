#ifndef GEN_ALLOC_DEFS_H
#define GEN_ALLOC_DEFS_H

#define GEN_ALLOC_ready(ta,type,field,len,a,i,n,x,base,ta_ready) \
int ta_ready(x,n) register ta *x; register unsigned int n; \
{ register unsigned int i; \
  unsigned int nlen; \
  if (x->field) { \
    i = x->a; \
    if (n > i) { \
      unsigned int nnum; \
      if (__builtin_add_overflow(base, n, &nlen)) \
        return 0; \
      if (__builtin_add_overflow(nlen, n >> 3, &nlen)) \
        return 0; \
      if (__builtin_mul_overflow(nlen, sizeof(type), &nnum)) \
        return 0; \
      x->a = nlen; \
      if (alloc_re(&x->field,i * sizeof(type),nnum)) return 1; \
      x->a = i; return 0; } \
    return 1; } \
  x->len = 0; \
  return !!(x->field = (type *) alloc((x->a = n) * sizeof(type))); }

#define GEN_ALLOC_readyplus(ta,type,field,len,a,i,n,x,base,ta_rplus) \
int ta_rplus(x,n) register ta *x; unsigned int n; \
{ register unsigned int i; \
  if (x->field) { \
    i = x->a; n += x->len; \
    if (__builtin_add_overflow(n, x->len, &n)) \
      return 0; \
    if (n > i) { \
      unsigned int nlen, nnum; \
      if (__builtin_add_overflow(base, n, &nlen)) \
        return 0; \
      if (__builtin_add_overflow(nlen, n >> 3, &nlen)) \
        return 0; \
      if (__builtin_mul_overflow(nlen, sizeof(type), &nnum)) \
        return 0; \
      x->a = nlen; \
      if (alloc_re(&x->field,i * sizeof(type),nnum)) return 1; \
      x->a = i; return 0; } \
    return 1; } \
  x->len = 0; \
  return !!(x->field = (type *) alloc((x->a = n) * sizeof(type))); }

#define GEN_ALLOC_append(ta,type,field,len,a,i,n,x,base,ta_rplus,ta_append) \
int ta_append(x,i) register ta *x; register type *i; \
{ if (!ta_rplus(x,1)) return 0; x->field[x->len++] = *i; return 1; }

#endif
