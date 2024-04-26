#include <check.h>

#include "byte.h"

START_TEST(test_byte_copy)
{
  char buf[8] = "abc def";
  byte_copy(buf, 2, "ddd");
  ck_assert_mem_eq(buf, "ddc def", 8);
  byte_copy(buf, 2, "ab");
  ck_assert_mem_eq(buf, "abc def", 8);
  byte_copy(buf, 2, "a");
  ck_assert_mem_eq(buf, "a\0c def", 8);
  byte_copy(buf, 8, "\0\1\2\3\4\5\6\7");
  ck_assert_mem_eq(buf, "\0\1\2\3\4\5\6\7", 8);
}
END_TEST

TCase
*str_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_byte_copy);

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
