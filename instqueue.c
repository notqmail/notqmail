#include "hier.h"

#include <sys/stat.h>
#include <stddef.h>
#include "open.h"
#include "strerr.h"

int main(void)
{
  if (open_read(".") == -1)
    strerr_die1sys(111,"instqueue: fatal: unable to open current directory: ");

  umask(077);
  hier_queue();
  return 0;
}
