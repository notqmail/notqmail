#include "substdio.h"
#include "byte.h"
#include "error.h"

static int allwrite(ssize_t (*op)(int,const char*,size_t), int fd, const char *buf, size_t len)
{
  ssize_t w;

  while (len) {
    w = op(fd,buf,len);
    if (w == -1) {
      if (errno == error_intr) continue;
      return -1; /* note that some data may have been written */
    }
    /* if (w == 0), luser's fault */
    buf += w;
    len -= w;
  }
  return 0;
}

int substdio_flush(substdio *s)
{
  int p;
 
  p = s->p;
  if (!p) return 0;
  s->p = 0;
  return allwrite(s->op,s->fd,s->x,p);
}

int substdio_bput(substdio *s, const char *buf, size_t len)
{
  unsigned int n;
 
  while (len > (n = s->n - s->p)) {
    byte_copy(s->x + s->p,n,buf);
    s->p += n;
    buf += n;
    len -= n;
    if (substdio_flush(s) == -1) return -1;
  }
  /* now len <= s->n - s->p */
  byte_copy(s->x + s->p,len,buf);
  s->p += len;
  return 0;
}

int substdio_put(substdio *s, const char *buf, size_t len)
{
  unsigned int n = s->n; /* how many bytes to write in next chunk */
 
  /* check if the input would fit in the buffer without flushing */
  if (len > n - (unsigned int)s->p) {
    if (substdio_flush(s) == -1) return -1;
    /* now s->p == 0 */
    if (n < SUBSTDIO_OUTSIZE) n = SUBSTDIO_OUTSIZE;
    /* as long as the remainder would not fit into s->x write it directly
     * from buf to s->fd. */
    while (len > (unsigned int)s->n) {
      if (n > len) n = len;
      if (allwrite(s->op,s->fd,buf,n) == -1) return -1;
      buf += n;
      len -= n;
    }
  }
  /* now len <= s->n - s->p */
  byte_copy(s->x + s->p,len,buf);
  s->p += len;
  return 0;
}

int substdio_putflush(substdio *s, const char *buf, size_t len)
{
  if (substdio_flush(s) == -1) return -1;
  return allwrite(s->op,s->fd,buf,len);
}
