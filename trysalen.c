#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

void foo()
{
  struct sockaddr sa;
  sa.sa_len = 0;
}
