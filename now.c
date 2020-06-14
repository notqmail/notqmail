#include <time.h>
#include "datetime.h"
#include "now.h"

datetime_sec now()
{
  return time(NULL);
}
