/* 19950925 */
#include "caltime.h"
#include "datetime.h"
#include "tai.h"

void datetime_tai(struct datetime *dt, datetime_sec t)
{
  int yday;
  int wday;

  struct tai t2;
  struct caltime ct;

  tai_unix(&t2,t);
  caltime_utc(&ct,&t2,&wday,&yday);
 
  dt->hour = ct.hour;
  dt->min = ct.minute;
  dt->sec = ct.second;
  dt->wday = wday;
  dt->yday = yday;
  dt->year = ct.date.year - 1900;
  dt->mon = ct.date.month - 1;
  dt->mday = ct.date.day;
}
