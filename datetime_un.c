#include "datetime.h"

/* roughly 100x faster than mktime() */
datetime_sec datetime_untai(dt)
struct datetime *dt;
{
  int year;
  int day;
  int mon;
 
  year = dt->year + 1900;
 
  mon = dt->mon;
  if (mon >= 2) { mon -= 2; }
  else { mon += 10; --year; }
 
  day = (dt->mday - 1) * 10 + 5 + 306 * mon;
  day /= 10;
 
  if (day == 365) { year -= 3; day = 1460; }
  else { day += 365 * (year % 4); }
  year /= 4;
 
  day += 1461 * (year % 25);
  year /= 25;
 
  if (day == 36524) { year -= 3; day = 146096; }
  else { day += 36524 * (year % 4); }
  year /= 4;
 
  day += 146097 * (year - 5);
  day += 11017;
 
  return ((day * 24 + dt->hour) * 60 + dt->min) * 60 + dt->sec;
}
