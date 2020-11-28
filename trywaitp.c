#include <sys/types.h>
#include <sys/wait.h>
#include <stddef.h>

int main(void)
{
  return waitpid(0,NULL,0);
}
