#include <sys/types.h>
#include <sys/wait.h>

void main(void)
{
  waitpid(0,0,0);
}
