#include "substdio.h"

void substdio_fdbuf(substdio *s, ssize_t (*op)(), int fd, char *buf, int len)
{
  s->x = buf;
  s->fd = fd;
  s->op = op;
  s->p = 0;
  s->n = len;
}
