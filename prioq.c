#include "alloc.h"
#include "gen_allocdefs.h"
#include "prioq.h"

GEN_ALLOC_readyplus(prioq,struct prioq_elt,p,len,a,i,n,x,100,prioq_readyplus)

int prioq_insert(pq,pe)
prioq *pq;
struct prioq_elt *pe;
{
 int i;
 int j;
 if (!prioq_readyplus(pq,1)) return 0;
 j = pq->len++;
 while (j)
  {
   i = (j - 1)/2;
   if (pq->p[i].dt <= pe->dt) break;
   pq->p[j] = pq->p[i];
   j = i;
  }
 pq->p[j] = *pe;
 return 1;
}

int prioq_min(pq,pe)
prioq *pq;
struct prioq_elt *pe;
{
 if (!pq->p) return 0;
 if (!pq->len) return 0;
 *pe = pq->p[0];
 return 1;
}

void prioq_delmin(pq)
prioq *pq;
{
 int i;
 int j;
 int n;
 if (!pq->p) return;
 n = pq->len;
 if (!n) return;
 i = 0;
 --n;
 for (;;)
  {
   j = i + i + 2;
   if (j > n) break;
   if (pq->p[j - 1].dt <= pq->p[j].dt) --j;
   if (pq->p[n].dt <= pq->p[j].dt) break;
   pq->p[i] = pq->p[j];
   i = j;
  }
 pq->p[i] = pq->p[n];
 pq->len = n;
}
