#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h> /* SVR4 silliness */
#include <stddef.h>

int foo()
{
  return select(0, NULL, NULL, NULL, NULL);
}
