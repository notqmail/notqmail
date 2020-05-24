#include <check.h>

#include "str.h"

START_TEST(test_str_len)
{
  ck_assert_uint_eq(str_len(""), 0);
  ck_assert_uint_eq(str_len("notqmail"), 8);
}
END_TEST

START_TEST(test_str_diff)
{
  ck_assert_int_eq(str_diff("", ""), 0);

  ck_assert_int_ne(str_diff("", "a"), 0);

  ck_assert_int_ne(str_diff("b", "a"), 0);

  ck_assert_int_ne(str_diff("b", ""), 0);

  ck_assert_int_eq(str_diff("notqmail", "notqmail"), 0);

  ck_assert_int_ne(str_diff("netqmail", "notqmail"), 0);
}
END_TEST

START_TEST(test_str_diffn)
{
  ck_assert_int_eq(str_diffn("", "", 0), 0);
  ck_assert_int_eq(str_diffn("", "", 1), 0);

  ck_assert_int_eq(str_diffn("", "a", 0), 0);
  ck_assert_int_ne(str_diffn("", "a", 1), 0);

  ck_assert_int_eq(str_diffn("b", "a", 0), 0);
  ck_assert_int_ne(str_diffn("b", "a", 1), 0);

  ck_assert_int_eq(str_diffn("b", "", 0), 0);
  ck_assert_int_ne(str_diffn("b", "", 1), 0);

  ck_assert_int_eq(str_diffn("notqmail", "notqmail", 8), 0);
  ck_assert_int_eq(str_diffn("notqmail", "notqmail", 4), 0);

  ck_assert_int_eq(str_diffn("netqmail", "notqmail", 1), 0);
  ck_assert_int_ne(str_diffn("netqmail", "notqmail", 8), 0);
}
END_TEST

START_TEST(test_str_cpy)
{
  char outbuf[32];

  memset(outbuf, 'x', sizeof(outbuf));
  str_copy(outbuf, "");
  ck_assert_str_eq(outbuf, "");

  memset(outbuf, 'x', sizeof(outbuf));
  str_copy(outbuf, "notqmail");
  ck_assert_str_eq(outbuf, "notqmail");
}
END_TEST

TCase
*str_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_str_len);
  tcase_add_test(tc, test_str_diff);
  tcase_add_test(tc, test_str_diffn);
  tcase_add_test(tc, test_str_cpy);

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
