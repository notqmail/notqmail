#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
  return waitpid(0,0,0);
}
