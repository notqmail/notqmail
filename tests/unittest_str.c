#include <check.h>

#include "str.h"

START_TEST(test_str_chr)
{
  ck_assert_uint_eq(str_chr("abc", 'a'), 0);
  ck_assert_uint_eq(str_chr("abc", 'b'), 1);
  ck_assert_uint_eq(str_chr("abc", 'c'), 2);
  ck_assert_uint_eq(str_chr("abc", 'd'), 3);

  ck_assert_uint_eq(str_chr("", 'd'), 0);
}
END_TEST

TCase
*str_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_str_chr);

  return tc;
}

Suite
*str_suite(void)
{
  Suite *s = suite_create("notqmail str");

  suite_add_tcase(s, str_something());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(str_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
