#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>

void main(void)
{
  flock(0,LOCK_EX | LOCK_UN | LOCK_NB);
}
