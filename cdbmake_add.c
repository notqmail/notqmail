#include "cdbmake.h"

void cdbmake_init(cdbm)
struct cdbmake *cdbm;
{
  cdbm->head = 0;
  cdbm->split = 0;
  cdbm->hash = 0;
  cdbm->numentries = 0;
}

int cdbmake_add(cdbm,h,p,alloc)
struct cdbmake *cdbm;
uint32 h;
uint32 p;
char *(*alloc)();
{
  struct cdbmake_hplist *head;

  head = cdbm->head;
  if (!head || (head->num >= CDBMAKE_HPLIST)) {
    head = (struct cdbmake_hplist *) alloc(sizeof(struct cdbmake_hplist));
    if (!head) return 0;
    head->num = 0;
    head->next = cdbm->head;
    cdbm->head = head;
  }
  head->hp[head->num].h = h;
  head->hp[head->num].p = p;
  ++head->num;
  ++cdbm->numentries;
  return 1;
}

int cdbmake_split(cdbm,alloc)
struct cdbmake *cdbm;
char *(*alloc)();
{
  int i;
  uint32 u;
  uint32 memsize;
  struct cdbmake_hplist *x;

  for (i = 0;i < 256;++i)
    cdbm->count[i] = 0;

  for (x = cdbm->head;x;x = x->next) {
    i = x->num;
    while (i--)
      ++cdbm->count[255 & x->hp[i].h];
  }

  memsize = 1;
  for (i = 0;i < 256;++i) {
    u = cdbm->count[i] * 2;
    if (u > memsize)
      memsize = u;
  }

  memsize += cdbm->numentries; /* no overflow possible up to now */
  u = (uint32) 0 - (uint32) 1;
  u /= sizeof(struct cdbmake_hp);
  if (memsize > u) return 0;

  cdbm->split = (struct cdbmake_hp *) alloc(memsize * sizeof(struct cdbmake_hp));
  if (!cdbm->split) return 0;

  cdbm->hash = cdbm->split + cdbm->numentries;

  u = 0;
  for (i = 0;i < 256;++i) {
    u += cdbm->count[i]; /* bounded by numentries, so no overflow */
    cdbm->start[i] = u;
  }

  for (x = cdbm->head;x;x = x->next) {
    i = x->num;
    while (i--)
      cdbm->split[--cdbm->start[255 & x->hp[i].h]] = x->hp[i];
  }

  return 1;
}

uint32 cdbmake_throw(cdbm,pos,b)
struct cdbmake *cdbm;
uint32 pos;
int b;
{
  uint32 len;
  uint32 j;
  uint32 count;
  struct cdbmake_hp *hp;
  uint32 where;

  count = cdbm->count[b];

  len = count + count; /* no overflow possible */
  cdbmake_pack(cdbm->final + 8 * b,pos);
  cdbmake_pack(cdbm->final + 8 * b + 4,len);

  if (len) {
    for (j = 0;j < len;++j)
      cdbm->hash[j].h = cdbm->hash[j].p = 0;

    hp = cdbm->split + cdbm->start[b];
    for (j = 0;j < count;++j) {
      where = (hp->h >> 8) % len;
      while (cdbm->hash[where].p)
	if (++where == len)
	  where = 0;
      cdbm->hash[where] = *hp++;
    }
  }

  return len;
}
