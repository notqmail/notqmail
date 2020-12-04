#ifndef SUBSTDIO_H
#define SUBSTDIO_H

#include <sys/types.h>
#include <string.h>

#include "deprecated.h"

typedef struct substdio {
  char *x;
  int p;
  int n;
  int fd;
  ssize_t (*op)();
} substdio;

#define SUBSTDIO_FDBUF(o,f,buf,len) { (buf), 0, (len), (f), (o) }

extern void substdio_fdbuf(substdio *s, ssize_t (*op)(), int fd, char *buf, int len);

extern int substdio_flush();
extern int substdio_put(substdio *s, const char *buf, size_t len);
extern int substdio_bput(substdio *s, const char *buf, size_t len);
extern int substdio_putflush(substdio *s, const char *buf, size_t len);
static inline int substdio_puts(substdio *s, const char *buf)
{
  return substdio_put(s,buf,strlen(buf));
}
static inline int substdio_bputs(substdio *s, const char *buf)
{
  return substdio_bput(s,buf,strlen(buf));
}
static inline int substdio_putsflush(substdio *s, const char *buf)
{
  return substdio_putflush(s,buf,strlen(buf));
}

extern ssize_t substdio_get(substdio *s, char *buf, size_t len);
#ifdef DEPRECATED_FUNCTIONS_AVAILABLE
extern ssize_t substdio_bget(substdio *s, char *buf, size_t len);
#endif
extern ssize_t substdio_feed(substdio *s);

extern char *substdio_peek();
extern void substdio_seek();

#define substdio_fileno(s) ((s)->fd)

#define SUBSTDIO_INSIZE 8192
#define SUBSTDIO_OUTSIZE 8192

#define substdio_PEEK(s) ( (s)->x + (s)->n )
#define substdio_SEEK(s,len) ( ( (s)->p -= (len) ) , ( (s)->n += (len) ) )

#define substdio_BPUTC(s,c) \
  ( ((s)->n != (s)->p) \
    ? ( (s)->x[(s)->p++] = (c), 0 ) \
    : substdio_bput((s),&(c),1) \
  )

extern int substdio_copy();

#endif
