#include <check.h>

#include "byte.h"

START_TEST(test_byte_equal)
{
  ck_assert_int_ne(byte_equal("", 0, ""), 0);
  ck_assert_int_ne(byte_equal("", 1, ""), 0);

  ck_assert_int_ne(byte_equal("", 0, "a"), 0);
  ck_assert_int_eq(byte_equal("", 1, "a"), 0);

  ck_assert_int_ne(byte_equal("b", 0, "a"), 0);
  ck_assert_int_eq(byte_equal("b", 1, "a"), 0);

  ck_assert_int_ne(byte_equal("b", 0, ""), 0);
  ck_assert_int_eq(byte_equal("b", 1, ""), 0);

  ck_assert_int_ne(byte_equal("notqmail", 8, "notqmail"), 0);
  ck_assert_int_ne(byte_equal("notqmail", 9, "notqmail"), 0);
  ck_assert_int_ne(byte_equal("notqmail", 4, "notqmail"), 0);

  ck_assert_int_ne(byte_equal("netqmail", 1, "notqmail"), 0);
  ck_assert_int_eq(byte_equal("netqmail", 8, "notqmail"), 0);

  ck_assert_int_ne(byte_equal("a\0b\0", 3, "a\0bc"), 0);
  ck_assert_int_eq(byte_equal("a\0b\0", 4, "a\0bc"), 0);
}
END_TEST

TCase
*str_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_byte_equal);

  return tc;
}

Suite
*str_suite(void)
{
  Suite *s = suite_create("notqmail byte");

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
