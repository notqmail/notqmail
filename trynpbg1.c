#include "select.h"
#include "open.h"
#include "fifo.h"

#define FN "temp-trynpbg1.fifo"

void main()
{
  int flagbug;
  struct timeval instant;
  fd_set rfds;
 
  flagbug = 0;
  if (fifo_make(FN,0600) != -1) {
    close(0);
    if (open_read(FN) == 0) {
      FD_ZERO(&rfds);
      FD_SET(0,&rfds);
      instant.tv_sec = instant.tv_usec = 0;
      if (select(1,&rfds,(fd_set *) 0,(fd_set *) 0,&instant) > 0)
        flagbug = 1;
    }
    unlink(FN);
  }
  _exit(!flagbug);
}
