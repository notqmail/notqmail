#include <check.h>

#define DEPRECATED_FUNCTIONS_REMOVED
#include "qmail-send-without-main.c"

extern struct job *jo;
extern int numjobs;

// START test helpers for the API we might prefer from qmail-send

#define available 0
#define unavailable 1

static void job_init_this_many(int numslots)
{
  numjobs = numslots;
  job_init();
}

static void occupy_job(unsigned int index)
{
  jo[index].refs = unavailable;
}

static void occupy_all_jobs(void)
{
  int j;

  for (j = 0; j < numjobs; j++)
  {
    occupy_job(j);
  }
}

static void release_job(unsigned int index)
{
  jo[index].refs = available;
}

// END test helpers for the API we might prefer from qmail-send

// when job_avail() returns an index, switch to ck_assert_int_eq
#define assert_job_avail ck_assert_uint_eq

static void assert_job_at_index(unsigned int index, int availability)
{
  ck_assert_int_eq(jo[index].refs, availability);
}

static void setup(void)
{
  numjobs = -77;
}

static void teardown(void)
{
  numjobs = -88;
}

static void assert_all_slots_are_available(int numjobs)
{
  int j;

  for (j = 0; j < numjobs; j++)
  {
    assert_job_at_index(j, available);
    ck_assert_ptr_null(jo[j].sender.s);
  }
}

START_TEST(test_qmail_job_init_sets_expected_default_state)
{
  ck_assert_ptr_null(jo);

  job_init_this_many(4);

  ck_assert_ptr_nonnull(jo);
  ck_assert_uint_eq(numjobs, 4);
  assert_all_slots_are_available(4);
}
END_TEST

START_TEST(test_qmail_job_avail_returns_free_slots_after_init)
{
  job_init_this_many(5);
  assert_job_avail(job_avail(), 1);
}
END_TEST

START_TEST(test_qmail_job_avail_returns_no_free_slots)
{
  job_init_this_many(5);
  occupy_all_jobs();

  assert_job_avail(job_avail(), 0);
}
END_TEST

START_TEST(test_qmail_job_avail_returns_a_free_slot)
{
  job_init_this_many(5);
  occupy_all_jobs();
  release_job(2);

  assert_job_avail(job_avail(), 1);
}
END_TEST

START_TEST(test_qmail_job_avail_returns_a_free_slot_for_index_zero)
{
  job_init_this_many(5);
  occupy_all_jobs();
  release_job(0);

  assert_job_avail(job_avail(), 1);
}
END_TEST

START_TEST(test_qmail_job_avail_returns_a_free_slot_for_last_index)
{
  job_init_this_many(5);
  occupy_all_jobs();
  release_job(numjobs - 1);

  assert_job_avail(job_avail(), 1);
}
END_TEST

START_TEST(test_qmail_job_open_slots_full)
{
  int index;

  job_init_this_many(2);
  occupy_all_jobs();

  index = job_open(1,2);

  ck_assert_int_eq(index, -1);
}
END_TEST

START_TEST(test_qmail_job_open_slot_available)
{
  unsigned long job_open_id;
  int job_open_channel;
  int index;

  job_init_this_many(5);
  occupy_all_jobs();
  release_job(3);
  job_open_id = 6546;
  job_open_channel = 5;

  index = job_open(job_open_id,job_open_channel);

  ck_assert_int_eq(index, 3);
  assert_job_at_index(3, unavailable);
  ck_assert_uint_eq(jo[3].id, job_open_id);
  ck_assert_int_eq(jo[3].channel, job_open_channel);
  ck_assert_int_eq(jo[3].numtodo, 0);
  ck_assert_int_eq(jo[3].flaghiteof, 0);
}
END_TEST

START_TEST(test_qmail_job_open_multiple_slots_available)
{
  unsigned long job_open_id;
  int job_open_channel;
  int index;

  job_init_this_many(7);
  occupy_all_jobs();
  release_job(2);
  release_job(3);
  job_open_id = 7890;
  job_open_channel = 572;

  index = job_open(job_open_id,job_open_channel);

  ck_assert_int_eq(index, 2);
  assert_job_at_index(2, unavailable);
  ck_assert_uint_eq(jo[2].id, job_open_id);
  ck_assert_int_eq(jo[2].channel, job_open_channel);
  ck_assert_int_eq(jo[2].numtodo, 0);
  ck_assert_int_eq(jo[2].flaghiteof, 0);
}
END_TEST

START_TEST(test_qmail_job_close_decrements_specified_full_slot)
{
  unsigned long job_open_id;
  int job_open_channel;
  int index;

  job_init_this_many(7);
  occupy_all_jobs();
  release_job(5);
  release_job(6);
  job_open_id = 79;
  job_open_channel = 36;

  index = job_open(job_open_id, job_open_channel);

  ck_assert_int_eq(index, 5);
  assert_job_at_index(index, unavailable);
  job_close(index);
  assert_job_at_index(index, available);
  job_close(index);
  ck_assert_int_lt(jo[index].refs, available);
}
END_TEST

TCase *
qmail_send_test_job_allocations(void)
{
  TCase *tc = tcase_create("test job allocations");

  tcase_add_checked_fixture(tc, setup, teardown);
  tcase_add_test(tc, test_qmail_job_init_sets_expected_default_state);
  tcase_add_test(tc, test_qmail_job_avail_returns_free_slots_after_init);
  tcase_add_test(tc, test_qmail_job_avail_returns_no_free_slots);
  tcase_add_test(tc, test_qmail_job_avail_returns_a_free_slot);
  tcase_add_test(tc, test_qmail_job_avail_returns_a_free_slot_for_last_index);
  tcase_add_test(tc, test_qmail_job_avail_returns_a_free_slot_for_index_zero);
  tcase_add_test(tc, test_qmail_job_open_slots_full);
  tcase_add_test(tc, test_qmail_job_open_slot_available);
  tcase_add_test(tc, test_qmail_job_open_multiple_slots_available);
  tcase_add_test(tc, test_qmail_job_close_decrements_specified_full_slot);

  return tc;
}

Suite *
qmail_send_suite(void)
{
  Suite *s = suite_create("notqmail qmail-send job");

  suite_add_tcase(s, qmail_send_test_job_allocations());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(qmail_send_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
