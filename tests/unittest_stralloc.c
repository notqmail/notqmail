#include <check.h>

#include "alloc.h"
#include "stralloc.h"

START_TEST(test_stralloc_thingy)
{
  stralloc thingy = {0};
  const char *input = "thingy";

  stralloc_copys(&thingy,input);
  stralloc_0(&thingy);

  ck_assert_str_eq(input, thingy.s);

  alloc_free(thingy.s);
}
END_TEST

TCase
*stralloc_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_stralloc_thingy);

  return tc;
}

Suite
*stralloc_suite(void)
{
  Suite *s = suite_create("notqmail stralloc");

  suite_add_tcase(s, stralloc_something());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(stralloc_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
