#include <sys/types.h>
#include <utime.h>

void foo()
{
  struct utimbuf ut;
  ut.actime = ut.modtime = 42;
  utime("/dummy", &ut);
}
