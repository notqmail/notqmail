#include <sys/types.h>
#include <sys/stat.h>

void main(void)
{
  mkfifo("temp-trymkffo",0);
}
