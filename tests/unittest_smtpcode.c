#include <assert.h>
#include <check.h>
#include <stdlib.h>

#include "smtpcode.h"
#include "stralloc.h"
#include "substdio.h"

static const char *bytes_from_remote;
static size_t bytes_read;

void temp_nomem()
{
  abort();
}

static int readstub(int fd, char *buf, int len)
{
  size_t bytes_to_read = strlen(bytes_from_remote) - bytes_read;

  ck_assert_int_eq(fd, -1);
  ck_assert_int_gt(len, 0);

  if (len < bytes_to_read)
    bytes_to_read = len;

  if (bytes_to_read > 0) {
    memcpy(buf, bytes_from_remote + bytes_read, bytes_to_read);
    bytes_read += bytes_to_read;
  }

  return bytes_to_read;
}

char smtpfrombuf[128];
substdio smtpfrom = SUBSTDIO_FDBUF(readstub,-1,smtpfrombuf,sizeof(smtpfrombuf));

stralloc smtptext = {0};

static unsigned long run_one(const char *text)
{
  unsigned long r;

  bytes_from_remote = text;
  bytes_read = 0;

  /* poison the buffer with invalid stuff, must be overwritten */
  if (!stralloc_copys(&smtptext,"junk"))
    temp_nomem();

  r = smtpcode();

  /* 0-terminate the contents */
  if (!stralloc_0(&smtptext))
    temp_nomem();

  ck_assert_uint_eq(bytes_read, strlen(bytes_from_remote));

  return r;
}

static unsigned long run_one_good(const char *text)
{
  char ebuf[256];
  unsigned long i, r;

  /* copy without the '\r' */
  r = 0;
  for (i = 0; i < strlen(text); i++) {
    if (text[i] != '\r') {
      ebuf[r++] = text[i];
    }
    ck_assert_uint_lt(r, sizeof(ebuf));
  }
  ebuf[r] = '\0';

  r = run_one(text);

  ck_assert_uint_lt(smtptext.len, sizeof(ebuf));
  ck_assert_str_eq(smtptext.s, ebuf);

  return r;
}

START_TEST(test_smtpcode_250)
{
  ck_assert_uint_eq(run_one_good("250 \r\n"), 250);
}
END_TEST

START_TEST(test_smtpcode_250_multiline)
{
  ck_assert_uint_eq(run_one_good("250-\r\n250 \r\n"), 250);
}
END_TEST

START_TEST(test_smtpcode_400)
{
  ck_assert_uint_eq(run_one_good("400 \r\n"), 400);
}
END_TEST

START_TEST(test_smtpcode_500)
{
  ck_assert_uint_eq(run_one_good("500 \r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_567)
{
  ck_assert_uint_eq(run_one_good("567 \r\n"), 567);
}
END_TEST

START_TEST(test_smtpcode_empty)
{
  ck_assert_uint_gt(run_one("\r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_dot)
{
  ck_assert_uint_gt(run_one("250.\r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_low)
{
  ck_assert_uint_gt(run_one("180\r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_1000)
{
  ck_assert_uint_gt(run_one("1000\r\n"), 999);
}
END_TEST

START_TEST(test_smtpcode_noterm)
{
  ck_assert_uint_gt(run_one("250\r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_badterm)
{
  ck_assert_uint_gt(run_one("250x\r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_badterm_multiline)
{
  ck_assert_uint_gt(run_one("250-\r\n250x\r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_badcode_multiline)
{
  ck_assert_uint_gt(run_one("250-\r\n2X0 \r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_diffcode_multiline1)
{
  ck_assert_uint_gt(run_one("250-\r\n350 \r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_diffcode_multiline2)
{
  ck_assert_uint_gt(run_one("250-\r\n260 \r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_diffcode_multiline3)
{
  ck_assert_uint_gt(run_one("250-\r\n251 \r\n"), 500);
}
END_TEST

START_TEST(test_smtpcode_letters)
{
  /* given the ASCII codes this is < 250 */
  ck_assert_uint_gt(run_one("0AB  \r\n"), 500);
}
END_TEST

TCase
*smtpcode_good(void)
{
  TCase *tc = tcase_create("good inputs");

  tcase_add_test(tc, test_smtpcode_250);
  tcase_add_test(tc, test_smtpcode_250_multiline);
  tcase_add_test(tc, test_smtpcode_400);
  tcase_add_test(tc, test_smtpcode_500);
  tcase_add_test(tc, test_smtpcode_567);

  return tc;
}

TCase
*smtpcode_bad(void)
{
  TCase *tc = tcase_create("invalid inputs");

  tcase_add_test(tc, test_smtpcode_empty);
  tcase_add_test(tc, test_smtpcode_dot);
  tcase_add_test(tc, test_smtpcode_low);
  tcase_add_test(tc, test_smtpcode_1000);
  tcase_add_test(tc, test_smtpcode_noterm);
  tcase_add_test(tc, test_smtpcode_badterm);
  tcase_add_test(tc, test_smtpcode_badterm_multiline);
  tcase_add_test(tc, test_smtpcode_badcode_multiline);
  tcase_add_test(tc, test_smtpcode_diffcode_multiline1);
  tcase_add_test(tc, test_smtpcode_diffcode_multiline2);
  tcase_add_test(tc, test_smtpcode_diffcode_multiline3);
  tcase_add_test(tc, test_smtpcode_letters);

  return tc;
}
Suite
*smtpcode_suite(void)
{
  Suite *s = suite_create("notqmail qmail-remote smtpcode");

  suite_add_tcase(s, smtpcode_good());
  suite_add_tcase(s, smtpcode_bad());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(smtpcode_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
