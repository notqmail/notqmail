#include "substdio.h"
#include "byte.h"
#include "error.h"

static ssize_t oneread(ssize_t (*op)(), int fd, char *buf, size_t len)
{
  for (;;) {
    ssize_t r = op(fd,buf,len);
    if (r == -1) if (errno == error_intr) continue;
    return r;
  }
}

static int getthis(s,buf,len)
register substdio *s;
register char *buf;
register int len;
{
  register int r;
  register int q;
 
  r = s->p;
  q = r - len;
  if (q > 0) { r = len; s->p = q; } else s->p = 0;
  byte_copy(buf,r,s->x + s->n);
  s->n += r;
  return r;
}

ssize_t substdio_feed(substdio *s)
{
  ssize_t r;
  register int q;

  if (s->p) return s->p;
  q = s->n;
  r = oneread(s->op,s->fd,s->x,q);
  if (r == 0 || r == -1) return r;
  s->p = r;
  q -= r;
  s->n = q;
  if (q > 0) /* damn, gotta shift */ byte_copyr(s->x + q,r,s->x);
  return r;
}

#ifdef DEPRECATED_FUNCTIONS_AVAILABLE
// might read fewer bytes than _get(); otherwise identical
ssize_t substdio_bget(substdio *s, char *buf, size_t len)
{
  ssize_t r;
 
  if (s->p > 0) return getthis(s,buf,len);
  r = s->n; if (r <= len) return oneread(s->op,s->fd,buf,r);
  r = substdio_feed(s);
  if (r == 0 || r == -1) return r;
  return getthis(s,buf,len);
}
#endif

ssize_t substdio_get(substdio *s, char *buf, size_t len)
{
  ssize_t r;
 
  if (s->p > 0) return getthis(s,buf,len);
  r = s->n; if (r <= len) return oneread(s->op,s->fd,buf,len);
  r = substdio_feed(s);
  if (r == 0 || r == -1) return r;
  return getthis(s,buf,len);
}

char *substdio_peek(s)
register substdio *s;
{
  return s->x + s->n;
}

void substdio_seek(s,len)
register substdio *s;
register int len;
{
  s->n += len;
  s->p -= len;
}
