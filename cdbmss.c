#include "readwrite.h"
#include "seek.h"
#include "alloc.h"
#include "cdbmss.h"

int cdbmss_start(c,fd)
struct cdbmss *c;
int fd;
{
  cdbmake_init(&c->cdbm);
  c->fd = fd;
  c->pos = sizeof(c->cdbm.final);
  substdio_fdbuf(&c->ss,write,fd,c->ssbuf,sizeof(c->ssbuf));
  return seek_set(fd,(seek_pos) c->pos);
}

int cdbmss_add(c,key,keylen,data,datalen)
struct cdbmss *c;
unsigned char *key;
unsigned int keylen;
unsigned char *data;
unsigned int datalen;
{
  uint32 h;
  int i;

  cdbmake_pack(c->packbuf,(uint32) keylen);
  cdbmake_pack(c->packbuf + 4,(uint32) datalen);
  if (substdio_put(&c->ss,c->packbuf,8) == -1) return -1;
  if (substdio_put(&c->ss,key,keylen) == -1) return -1;
  if (substdio_put(&c->ss,data,datalen) == -1) return -1;

  h = CDBMAKE_HASHSTART;
  for (i = 0;i < keylen;++i)
    h = cdbmake_hashadd(h,(unsigned int) key[i]);

  if (!cdbmake_add(&c->cdbm,h,c->pos,alloc)) return -1;

  c->pos += 8 + keylen + datalen; /* XXX: overflow? */
  return 0;
}

int cdbmss_finish(c)
struct cdbmss *c;
{
  int i;
  uint32 len;
  uint32 u;

  if (!cdbmake_split(&c->cdbm,alloc)) return -1;

  for (i = 0;i < 256;++i) {
    len = cdbmake_throw(&c->cdbm,c->pos,i);
    for (u = 0;u < len;++u) {
      cdbmake_pack(c->packbuf,c->cdbm.hash[u].h);
      cdbmake_pack(c->packbuf + 4,c->cdbm.hash[u].p);
      if (substdio_put(&c->ss,c->packbuf,8) == -1) return -1;
      c->pos += 8; /* XXX: overflow? */
    }
  }

  if (substdio_flush(&c->ss) == -1) return -1;
  if (seek_begin(c->fd) == -1) return -1;
  return substdio_putflush(&c->ss,c->cdbm.final,sizeof(c->cdbm.final));
}
