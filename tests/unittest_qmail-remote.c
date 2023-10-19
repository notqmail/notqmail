#include <check.h>

#include "substdio.h"

/* provided by qmail-remote.c/blast.c */
extern void blast();
extern char inbuf[1024];
extern substdio ssin;
extern char smtptobuf[1024];
extern substdio smtpto;

static const char *readexpect;
static const char *writeexpect;
static size_t readoffs;
static size_t writeoffs;

ssize_t readstub(int fd, char *buf, size_t len)
{
  size_t inlen = strlen(readexpect) - readoffs;

  ck_assert_int_eq(fd, -1);
  ck_assert_int_gt(len, 0);

  if (len < inlen)
    inlen = len;

  if (inlen > 0) {
    memcpy(buf, readexpect + readoffs, inlen);
    readoffs += inlen;
  }

  return inlen;
}

ssize_t writestub(int fd, const char *buf, size_t len)
{
  size_t outlen = strlen(writeexpect) - writeoffs;

  ck_assert_int_eq(fd, -1);
  ck_assert_int_gt(len, 0);

  if (len < outlen)
    outlen = len;

#if (CHECK_MAJOR_VERSION > 0) || (CHECK_MINOR_VERSION >= 11)
  ck_assert_mem_eq(writeexpect + writeoffs, buf, outlen);
#else
  ck_assert_int_eq(memcmp(writeexpect + writeoffs, buf, outlen), 0);
#endif

  writeoffs += outlen;

  return outlen;
}

static void ssin_setup(const char *indata, const char *outdata)
{
  substdio tmpin = SUBSTDIO_FDBUF(readstub,-1,inbuf,sizeof(inbuf));
  substdio tmpto = SUBSTDIO_FDBUF(writestub,-1,smtptobuf,sizeof(smtptobuf));
  ssin = tmpin;
  smtpto = tmpto;

  readexpect = indata;
  readoffs = 0;
  writeexpect = outdata;
  writeoffs = 0;
}

START_TEST(test_blast_empty)
{
  const char *dotcrlf = ".\r\n";

  ssin_setup("", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

START_TEST(test_blast_dot)
{
  const char *dotcrlf = "..\r\n.\r\n";

  ssin_setup(".\n", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

START_TEST(test_blast_barecr)
{
  const char *dotcrlf = "cr\r\nlf\r\n.\r\n";

  ssin_setup("cr\rlf\n", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

START_TEST(test_blast_crlf)
{
  const char *dotcrlf = "..\r\n.\r\n";

  ssin_setup(".\r\n", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

TCase
*blast_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_blast_empty);
  tcase_add_test(tc, test_blast_dot);
  tcase_add_test(tc, test_blast_barecr);
  tcase_add_test(tc, test_blast_crlf);

  return tc;
}

Suite
*blast_suite(void)
{
  Suite *s = suite_create("notqmail qmail-remote blast");

  suite_add_tcase(s, blast_something());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(blast_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
