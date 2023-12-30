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

START_TEST(test_stralloc_sizes)
{
  stralloc thingy = {0};
  const char *input = "thingy";
  unsigned int olen;

  // make room for exactly 42 bytes
  int r = stralloc_ready(&thingy, 42);
  ck_assert_int_eq(r, 1);
  ck_assert_uint_eq(thingy.len, 0);
  ck_assert_uint_eq(thingy.a, 42);
  // make sure that there is room for 42 more bytes
  // given that there are 42 bytes in it and nothing is used this does nothing
  r = stralloc_readyplus(&thingy, 42);
  ck_assert_int_eq(r, 1);
  ck_assert_uint_eq(thingy.len, 0);
  ck_assert_uint_eq(thingy.a, 42);
  // make sure there is room for 16 more bytes
  // there is already more room available, so this does nothing
  r = stralloc_readyplus(&thingy, 16);
  ck_assert_int_eq(r, 1);
  ck_assert_uint_eq(thingy.len, 0);
  ck_assert_uint_eq(thingy.a, 42);

  r = stralloc_copys(&thingy,input);
  ck_assert_int_eq(r, 1);
  ck_assert_uint_eq(thingy.len, strlen(input));
  ck_assert_uint_eq(thingy.a, 42);

  r = stralloc_0(&thingy);
  ck_assert_int_eq(r, 1);
  ck_assert_uint_eq(thingy.len, strlen(input) + 1);
  ck_assert_uint_eq(thingy.a, 42);

  // make sure that there is room for 42 more bytes
  r = stralloc_readyplus(&thingy, 42);
  ck_assert_int_eq(r, 1);
  ck_assert_uint_eq(thingy.len, strlen(input) + 1);
  ck_assert_uint_ge(thingy.a, thingy.len + 42);
  olen = thingy.a;

  // another stralloc_ready should not touch anything
  r = stralloc_ready(&thingy, 42);
  ck_assert_int_eq(r, 1);
  ck_assert_uint_eq(thingy.len, strlen(input) + 1);
  ck_assert_uint_ge(thingy.a, olen);

  unsigned int ofl = -30;

  r = stralloc_readyplus(&thingy, ofl);
  ck_assert_int_eq(r, 0);
  ck_assert_uint_eq(thingy.len, strlen(input) + 1);
  ck_assert_uint_ge(thingy.a, olen);

  ck_assert_str_eq(input, thingy.s);

  alloc_free(thingy.s);
}
END_TEST

TCase
*stralloc_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_stralloc_thingy);
  tcase_add_test(tc, test_stralloc_sizes);

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
