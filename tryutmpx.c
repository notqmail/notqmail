#include <utmpx.h>

int main(void)
{
  struct utmpx ut;
  ut.ut_type = sizeof(ut.ut_line) + sizeof(ut.ut_user);
  return 0;
}
