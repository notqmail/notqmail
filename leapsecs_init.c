#include "leapsecs.h"

#include "open.h"

#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

struct tai *leapsecs = 0;
int leapsecs_num = 0;

static int leapsecs_read()
{
  int fd;
  struct stat st;
  struct tai *t;
  int n;
  int i;

  fd = open_read("/etc/leapsecs.dat");
  if (fd == -1) {
    if (errno != ENOENT) return -1;
    free(leapsecs);
    leapsecs = NULL;
    leapsecs_num = 0;
    return 0;
  }

  if (fstat(fd,&st) == -1) {
    close(fd);
    return -1;
  }

  t = malloc(st.st_size);
  if (!t) {
    close(fd);
    return -1;
  }

  n = read(fd,t,st.st_size);
  close(fd);
  if (n != st.st_size) { free(t); return -1; }

  n /= sizeof(struct tai);

  for (i = 0;i < n;++i) {
    struct tai u;
    tai_unpack((char *) &t[i],&u);
    t[i] = u;
  }

  free(leapsecs);

  leapsecs = t;
  leapsecs_num = n;

  return 0;
}

int leapsecs_init()
{
  static int flaginit;
  if (flaginit) return 0;
  if (leapsecs_read() == -1) return -1;
  flaginit = 1;
  return 0;
}
