#include <string.h>

int main(void)
{
  char c;
  explicit_bzero(&c, 1);
}
