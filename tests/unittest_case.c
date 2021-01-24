#include <check.h>

#include "case.h"

START_TEST(test_case_diffs)
{
  ck_assert_uint_eq(case_diffs("abc", "abc"), 0);
  ck_assert_uint_ne(case_diffs("aba", "ab"), 0);
  ck_assert_uint_ne(case_diffs("ab", "aba"), 0);
  ck_assert_uint_eq(case_diffs("abc", "AbC"), 0);
  ck_assert_uint_eq(case_diffs("abc", "ABC"), 0);
  ck_assert_uint_eq(case_diffs("Abc", "abC"), 0);
  ck_assert_uint_ne(case_diffs("Abc", "BbC"), 0);
  ck_assert_uint_ne(case_diffs("Abc", "acC"), 0);
  ck_assert_uint_ne(case_diffs("Abc", "abd"), 0);
  ck_assert_uint_ne(case_diffs("ä", "Ä"), 0);
  ck_assert_uint_ne(case_diffs("", ""), 1);
}
END_TEST

TCase
*str_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_case_diffs);

  return tc;
}

Suite
*str_suite(void)
{
  Suite *s = suite_create("notqmail case");

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
