#include "readwrite.h"
#include "substdio.h"
#include "subfd.h"

int subfd_readsmall(fd,buf,len) int fd; char *buf; int len;
{
  if (substdio_flush(subfdoutsmall) == -1) return -1;
  return read(fd,buf,len);
}

char subfd_inbufsmall[256];
static substdio it = SUBSTDIO_FDBUF(subfd_readsmall,0,subfd_inbufsmall,256);
substdio *subfdinsmall = &it;
