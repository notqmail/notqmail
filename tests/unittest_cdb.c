#include <check.h>

#include "alloc.h"
#include "cdb.h"

START_TEST(test_cdb_hash)
{
  ck_assert_uint_eq(cdb_hash("abc", 1), 177604);
  ck_assert_uint_eq(cdb_hash("abc", 2), 5860902);
  ck_assert_uint_eq(cdb_hash("abc", 3), 193409669);
  ck_assert_uint_eq(cdb_hash("notqmail", 8), 3829944712);
}
END_TEST

START_TEST(test_cdb_unpack)
{
  const char fourtytwo[] = { 0x42, 0x42, 0x42, 0x42 };
  const char hash1[] = { 0xff, 0x42, 0x42, 0x42 };
  const char hash2[] = { 0x42, 0x42, 0x42, 0xff };

  ck_assert_uint_eq(cdb_unpack(fourtytwo), 0x42424242);
  ck_assert_uint_eq(cdb_unpack(hash1), 0x424242ff);
  ck_assert_uint_eq(cdb_unpack(hash2), 0xff424242);
}
END_TEST

TCase
*cdb_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_cdb_hash);
  tcase_add_test(tc, test_cdb_unpack);

  return tc;
}

Suite
*cdb_suite(void)
{
  Suite *s = suite_create("notqmail cdb");

  suite_add_tcase(s, cdb_something());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(cdb_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
