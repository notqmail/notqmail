/* 19950925 */
#include "datetime.h"

void datetime_tai(dt,t)
struct datetime *dt;
datetime_sec t;
{
  int day;
  int tod;
  int year;
  int yday;
  int wday;
  int mon;
 
  tod = t % 86400;
  day = t / 86400;
  if (tod < 0) { tod += 86400; --day; }
 
  dt->hour = tod / 3600;
  tod %= 3600;
  dt->min = tod / 60;
  dt->sec = tod % 60;
 
  wday = (day + 4) % 7; if (wday < 0) wday += 7;
  dt->wday = wday;
 
  day -= 11017;
  /* day 0 is march 1, 2000 */
  year = 5 + day / 146097;
  day = day % 146097; if (day < 0) { day += 146097; --year; }
  /* from now on, day is nonnegative */
  year *= 4;
  if (day == 146096) { year += 3; day = 36524; }
  else { year += day / 36524; day %= 36524; }
  year *= 25;
  year += day / 1461;
  day %= 1461;
  year *= 4;
  yday = (day < 306);
  if (day == 1460) { year += 3; day = 365; }
  else { year += day / 365; day %= 365; }
  yday += day;
 
  day *= 10;
  mon = (day + 5) / 306;
  day = day + 5 - 306 * mon;
  day /= 10;
  if (mon >= 10) { yday -= 306; ++year; mon -= 10; }
  else { yday += 59; mon += 2; }
 
  dt->yday = yday;
  dt->year = year - 1900;
  dt->mon = mon;
  dt->mday = day + 1;
}
