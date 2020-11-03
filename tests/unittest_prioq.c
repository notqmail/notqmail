#include <check.h>

#include <stdlib.h>

#include "prioq.h"

START_TEST(test_prioq_empty)
{
  prioq pq = {0};
  struct prioq_elt pe;

  // there's no minimum to return
  ck_assert_int_eq(0, prioq_min(&pq,&pe));
}
END_TEST

START_TEST(test_prioq_one_item)
{
  prioq pq = {0};
  struct prioq_elt pe;
  unsigned int count;

  pe.dt = 12345;
  pe.id = 77;
  prioq_insert(&pq,&pe);

  pe.dt = 0;
  pe.id = 0;

  count = 0;
  while (prioq_min(&pq,&pe)) {
    prioq_delmin(&pq);
    count++;
    // prioq_min stores priority and value in pe
    ck_assert_int_eq(12345, pe.dt);
    ck_assert_uint_eq(77, pe.id);
  }

  // there was exactly one entry in the queue
  ck_assert_uint_eq(count, 1);
}
END_TEST

START_TEST(test_prioq_insert_low_priority_to_high)
{
  prioq pq = {0};
  struct prioq_elt pe;
  unsigned long value;

  for (value = 0; value < 5; value++) {
    long priority = 12345 + value;
    pe.dt = priority;
    pe.id = value;
    ck_assert_int_ne(0, prioq_insert(&pq,&pe));
  }

  value = 0;
  // poison pe to make extra-sure prioq_min always overwrites
  pe.dt = -2; pe.id = 8;
  while (prioq_min(&pq,&pe)) {
    prioq_delmin(&pq);
    ck_assert_int_eq(12345 + value, pe.dt);
    ck_assert_uint_eq(value, pe.id);
    value++;
    // poison pe to make extra-sure prioq_min always overwrites
    pe.dt = -2; pe.id = 8;
  }

  // all 5 inserted entries were retrieved
  ck_assert_uint_eq(value, 5);
}
END_TEST

START_TEST(test_prioq_insert_high_priority_to_low)
{
  prioq pq = {0};
  struct prioq_elt pe;
  unsigned long value;

  for (value = 0; value < 5; value++) {
    long priority = 12345 - value;
    pe.dt = priority;
    pe.id = value;
    ck_assert_int_ne(0, prioq_insert(&pq,&pe));
  }

  // poison pe to make extra-sure prioq_min always overwrites
  pe.dt = -2; pe.id = 8;
  while (prioq_min(&pq,&pe)) {
    prioq_delmin(&pq);
    value--;
    ck_assert_int_eq(12345 - value, pe.dt);
    ck_assert_uint_eq(value, pe.id);
    // poison pe to make extra-sure prioq_min always overwrites
    pe.dt = -2; pe.id = 8;
  }

  // all 5 inserted entries were retrieved
  ck_assert_uint_eq(value, 0);
}
END_TEST

static int compare(const void *a, const void *b) {
  return (*(int*)a - *(int*)b);
}

START_TEST(test_prioq_insert_all_same_priority)
{
  prioq pq = {0};
  struct prioq_elt pe;
  long priority;
  unsigned int i;
  unsigned long sorted[5];
  unsigned long actual[5];

  priority = 12345;
  for (i = 0; i < 5; i++) {
    pe.dt = priority;
    pe.id = i;
    ck_assert_int_ne(0, prioq_insert(&pq,&pe));
    sorted[i] = i;
  }

  i = 0;
  // poison pe to make extra-sure prioq_min always overwrites
  pe.dt = -2; pe.id = 8;
  while (prioq_min(&pq,&pe)) {
    prioq_delmin(&pq);
    actual[i] = pe.id;
    i++;
    // poison pe to make extra-sure prioq_min always overwrites
    pe.dt = -2; pe.id = 8;
  }
  // all 5 inserted entries were retrieved
  ck_assert_uint_eq(i, 5);

  // no defined ordering, but at least all the same values are present
  qsort(actual, sizeof(actual)/sizeof(*actual), sizeof(*actual), compare);
  for (i = 0; i < 5; i++) {
    ck_assert_uint_eq(sorted[i], actual[i]);
  }
}
END_TEST

START_TEST(test_prioq_insert_no_particular_order)
{
  prioq pq = {0};
  struct prioq_elt pe;
  unsigned int i;
  long priority[5] = {123,234,-345,234,123};
  unsigned long value[5] = {77,66,88,55,99};
  long expected_priority[5] = {-345,123,123,234,234};
  unsigned long expected_value1[5] = {88,77,99,55,66}; // no defined ordering
  unsigned long expected_value2[5] = {88,99,77,66,55}; // see also below
  long actual_priority[5];
  unsigned long actual_value[5];

  for (i = 0; i < 5; i++) {
    pe.dt = priority[i];
    pe.id = value[i];
    ck_assert_int_ne(0, prioq_insert(&pq,&pe));
  }

  i = 0;
  // poison pe to make extra-sure prioq_min always overwrites
  pe.dt = -2; pe.id = 8;
  while (prioq_min(&pq,&pe)) {
    prioq_delmin(&pq);
    actual_priority[i] = pe.dt;
    actual_value[i] = pe.id;
    i++;
    // poison pe to make extra-sure prioq_min always overwrites
    pe.dt = -2; pe.id = 8;
  }

  for (i = 0; i < 5; i++) {
    ck_assert_int_eq(expected_priority[i], actual_priority[i]);

    // the first value for priority 123 may be either 77 or 99
    // for priority 234, either 55 or 66
    unsigned long expected_value = expected_value1[i];
    if (expected_value != actual_value[i]) expected_value = expected_value2[i];

    ck_assert_uint_eq(expected_value, actual_value[i]);
  }
}
END_TEST

TCase
*prioq_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_prioq_empty);
  tcase_add_test(tc, test_prioq_one_item);
  tcase_add_test(tc, test_prioq_insert_low_priority_to_high);
  tcase_add_test(tc, test_prioq_insert_high_priority_to_low);
  tcase_add_test(tc, test_prioq_insert_all_same_priority);
  tcase_add_test(tc, test_prioq_insert_no_particular_order);

  return tc;
}

Suite
*prioq_suite(void)
{
  Suite *s = suite_create("notqmail prioq");

  suite_add_tcase(s, prioq_something());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(prioq_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
