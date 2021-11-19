#include <check.h>

#include <stdlib.h>
#include <stdio.h>
#include "prioq.h"
#include "gen_allocdefs.h"

GEN_ALLOC_readyplus(prioq,struct prioq_elt,p,len,a,100,check_readyplus)

START_TEST(test_given_empty_prioq_should_return_0)
{
  prioq pq = {0};
  struct prioq_elt pe;

  // there's no minimum to return
  ck_assert_int_eq(0, prioq_min(&pq,&pe));
}
END_TEST

START_TEST(test_gen_alloc_readyplus_given_pq_empty_failed_and_set_a_to_2)
{
  prioq pq = {0};
  int return_code = check_readyplus(&pq, 2);

  ck_assert_int_eq(1, return_code);
  ck_assert_int_eq(2, pq.a);
}
END_TEST


START_TEST(test_prioq_one_item_insert_fetch_delete)
{
  prioq pq = {0};
  struct prioq_elt element;

  element.dt = 12345;
  element.id = 77;
  prioq_insert(&pq, &element);

  element.dt = 0;
  element.id = 0;

  unsigned int has_value = prioq_min(&pq, &element);
  ck_assert_uint_eq(has_value, 1);

  // prioq_min stores priority and value in element
  ck_assert_int_eq(12345, element.dt);
  ck_assert_uint_eq(77, element.id);
 
  // there is now exactly one entry in the queue
  prioq_delmin(&pq);
  ck_assert_uint_eq(prioq_min(&pq, &element), 0);
}
END_TEST

// We should make some helper functions
// - populating elements with our values
// - compare elements (assert) more generally way; extract method in line 47/8

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

START_TEST(test_prioq_min_given_insert_element_from_high_to_low_dt_with_same_id_should_return_pq_with_lowest_dt)
{
    prioq pq = {0};
    struct prioq_elt pe;

    unsigned long value;
    // insert the values that don't change the id value, but dt value is decreasing
    for (value = 0; value < 5; value++) {
        pe.dt = value;
        pe.id = 10;
        ck_assert_int_ne(0, prioq_insert(&pq,&pe));
    }

    // check to see if the element with the lowest dt is returned
    // if we have 2 values changed, which changes are considered in determine the priority of the queue
    // todo might need to add these comments to the existing tests
    // todo refactor the tests to make sure it's easy to understand from non-author dev (even for the author)
    int return_code = prioq_min(&pq,&pe);

    ck_assert_int_eq(pe.id, 10);
    ck_assert_int_eq(pe.dt, 0);
    ck_assert_int_eq(1, return_code);
}
END_TEST

TCase
*prioq_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_given_empty_prioq_should_return_0);
  tcase_add_test(tc, test_gen_alloc_readyplus_given_pq_empty_failed_and_set_a_to_2);

  tcase_add_test(tc, test_prioq_one_item_insert_fetch_delete);
  tcase_add_test(tc, test_prioq_insert_low_priority_to_high);
  tcase_add_test(tc, test_prioq_insert_high_priority_to_low);
  tcase_add_test(tc, test_prioq_insert_all_same_priority);
  tcase_add_test(tc, test_prioq_insert_no_particular_order);
  tcase_add_test(tc, test_prioq_min_given_insert_element_from_high_to_low_dt_with_same_id_should_return_pq_with_lowest_dt);

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
