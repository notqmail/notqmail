#include <sys/types.h>
#include <sys/time.h>
#include <syslog.h>

int
main()
{
  openlog("foo",0,LOG_MAIL);
  syslog(0,"foo");
  return 0;
}
