#include "datetime.h"
#include "fmt.h"
#include "myctime.h"

static char *daytab[7] = {
"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static char *montab[12] = {
"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static char result[30];

char *myctime(t)
datetime_sec t;
{
 struct datetime dt;
 unsigned int len;
 datetime_tai(&dt,t);
 len = 0;
 len += fmt_str(result + len,daytab[dt.wday]);
 result[len++] = ' ';
 len += fmt_str(result + len,montab[dt.mon]);
 result[len++] = ' ';
 len += fmt_uint0(result + len,dt.mday,2);
 result[len++] = ' ';
 len += fmt_uint0(result + len,dt.hour,2);
 result[len++] = ':';
 len += fmt_uint0(result + len,dt.min,2);
 result[len++] = ':';
 len += fmt_uint0(result + len,dt.sec,2);
 result[len++] = ' ';
 len += fmt_uint(result + len,1900 + dt.year);
 result[len++] = '\n';
 result[len++] = 0;
 return result;
}
