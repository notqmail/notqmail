#include <utmpx.h>

void main()
{
  struct utmpx ut;
  ut.ut_type = sizeof(ut.ut_line) + sizeof(ut.ut_user);
}
