#include <sys/types.h>
#include <sys/time.h>
#include <syslog.h>

main()
{
  openlog("foo",0,LOG_MAIL);
  syslog(0,"foo");
}
