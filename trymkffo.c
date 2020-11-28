#include <sys/types.h>
#include <sys/stat.h>

int main(void)
{
  return mkfifo("temp-trymkffo",0);
}
