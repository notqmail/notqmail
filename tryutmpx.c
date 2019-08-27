#include <utmpx.h>

void main(void)
{
  struct utmpx ut;
  ut.ut_type = sizeof(ut.ut_line) + sizeof(ut.ut_user);
}
