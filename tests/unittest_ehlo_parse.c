#include <check.h>

#include "ehlo_parse.h"
#include "stralloc.h"

static int bad_call(const char *ext, size_t extlen)
{
  (void)ext;
  (void)extlen;
  ck_abort();
  return 0;
}

static const struct smtpext bad_call_entry = {
  "server.example.org", bad_call
};

START_TEST(test_ehlo_noext)
{
  stralloc thingy = { 0 };
  unsigned int exts;
  thingy.s = (char*) "250 server.example.org\n";
  thingy.len = strlen(thingy.s);

  exts = ehlo_parse(&thingy, &bad_call_entry, 1);

  ck_assert_uint_eq(exts, 0);
}
END_TEST

START_TEST(test_ehlo_noparams)
{
  stralloc thingy = { 0 };
  unsigned int exts;
  thingy.s = (char*) "250-server.example.org\n"
                     "250-THEGOOD 1\n"
                     "250-THEBAD1\n"
                     "250 THEUGLY1\n";
  thingy.len = strlen(thingy.s);

  const struct smtpext callbacks[] = {
    bad_call_entry,
    { "THEGOOD", NULL },
    { "THEBAD", NULL },
    { "THEUGLY1", NULL }
  };

  exts = ehlo_parse(&thingy, callbacks, 4);

  ck_assert_uint_eq(exts, 8 | 2);
}
END_TEST

static int only_called_once(const char *ext, size_t extlen)
{
  static int guard;
  (void)ext;
  ck_assert_uint_eq(extlen, strlen("THEGOOD"));
  (void)extlen;
  if (guard++)
    ck_abort();
  return 1;
}

START_TEST(test_ehlo_nodupes)
{
  stralloc thingy = { 0 };
  unsigned int exts;
  thingy.s = (char*) "250-server.example.org\n"
                     "250-THEGOOD\n"
                     "250-THEGOOD\n"
                     "250 THEGOOD\n";
  thingy.len = strlen(thingy.s);

  const struct smtpext callback = {
    "THEGOOD", only_called_once
  };

  exts = ehlo_parse(&thingy, &callback, 1);

  ck_assert_uint_eq(exts, 1);
}
END_TEST

static int param_verifier(const char *ext, size_t extlen)
{
  static int guard;
  const char *params[] = {
    "THEGOOD a",
    "THEGOOD",
    "THEGOOD a b",
    "FINAL 1"
  };
  char buf[32];

  ck_assert_uint_eq(extlen, strlen(params[guard]));
  strncpy(buf, ext, extlen);
  buf[extlen] = '\0';
  ck_assert_str_eq(buf, params[guard]);
  guard++;

  return guard == 4 ? 1 : 0;
}

START_TEST(test_ehlo_params)
{
  stralloc thingy = { 0 };
  unsigned int exts;
  thingy.s = (char*) "250-server.example.org\n"
                     "250-THEGOOD a\n"
                     "250-THEGOOD\n"
                     "250-THEGOOD a b\n"
		     "250 FINAL 1";
  thingy.len = strlen(thingy.s);

  const struct smtpext callbacks[] = {
    { "THEGOOD", param_verifier },
    { "FINAL", param_verifier }
  };

  exts = ehlo_parse(&thingy, callbacks, 2);

  ck_assert_uint_eq(exts, 2);
}
END_TEST

TCase
*ehlo_parse_checks(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_ehlo_noext);
  tcase_add_test(tc, test_ehlo_noparams);
  tcase_add_test(tc, test_ehlo_nodupes);
  tcase_add_test(tc, test_ehlo_params);

  return tc;
}

Suite
*stralloc_suite(void)
{
  Suite *s = suite_create("notqmail ehlo_parse");

  suite_add_tcase(s, ehlo_parse_checks());

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
