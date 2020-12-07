#include <check.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "substdio.h"
#include "alloc.h"
#include "subfd.h"
#include "open.h"
#include "str.h"
#include "seek.h"
#include "utf8read.h"
#include "hassmtputf8.h"

int flagutf8, smtputf8;
stralloc smtptext = { 0 };
stralloc mfparms = {0};
char smtpfrombuf[128];

void out(s) char *s; { if (substdio_puts(subfdoutsmall,s) == -1) _exit(0); }
void temp_nomem() { out("Out of memory. (#4.3.0)\n"); }
void temp_read() { out("Unable to read message. (#4.3.0)\n"); }
void dropped() {out("ZConnected to xxxx but connection died (#4.4.2)\n");_exit(0);}

substdio smtpfrom = SUBSTDIO_FDBUF(read,0,smtpfrombuf,sizeof(smtpfrombuf));
extern int get_capability(const char *);
extern unsigned long smtpcode();
void mailfrom_parms(const char *);

char maildata_1[] =
  "Return-Path: <anonymous@argos.indimail.org>\n"
  "Delivered-To: mbhangui@argos.indimail.org\n"
  "Received: (indimail-mta 1842 invoked by alias); Thu, 3 Dec 2020 11:26:51 +0530\n"
  "Delivered-To: root@argos.indimail.org\n"
  "Received: (indimail-mta 3748 invoked by uid 0); Thu, 3 Dec 2020 10:30:00 +0530\n";

char maildata_2[] =
  "DKIM-Signature: v=1; a=rsa-sha1; c=relaxed/relaxed;\n"
  "  d=argos.indimail.org; s=default; x=1607576400; h=Date:Message-ID:\n"
  "  From:Subject:To; bh=pTv4MFW2fHcqP6Nreg/Zf8GFhtQ=; b=AD6ytMimMiPu\n"
  "  kdbP9+hbH8rQqIiUP5uU+125jwlESXVXG8xdMzVPspmP4K4Lo4KkcUR86xKpC0ee\n"
  "  Ak+T6fp3gqhZszbocYfag1YGIDTrc8fxmjf9Ycmg5BvGzxa+cEVs7CYfJ9hFZM7m\n"
  "  0AvoOdQ5fmWMCThNoHJzWDC3rTH7JFI=\n"
  "Date: Thu, 3 Dec 2020 10:29:33 +0530\n"
  "Message-ID: <20201203045933.3741.indimail@argos>\n"
  "From: anonymous@argos.indimail.org\n"
  "Subject: Output from your job      110\n"
  "To: root\n"
  "\n"
  "error: can't create transaction lock on /var/lib/rpm/.rpm.lock (Resource temporarily unavailable)\n"
  "error: /tmp/skype.gpgsig.zgIdKM: key 1 import failed.\n";

char *received[] = {
  /*- one long received string */
  "Received: from relay1.uu.net (HELO uunet.uu.net) (7@192.48.96.5) "
  "by silverton.berkeley.edu with UTF8SMTP; 29 Nov 2020 04:46:54 -0000\n",
  /*- with folded received header */
  "Received: from relay1.uu.net (HELO uunet.uu.net) (7@192.48.96.5)\n" 
  " by silverton.berkeley.edu with UTF8SMTP; 29 Nov 2020 04:46:54 -0000\n",
  /*- with folded received header and extra space */
  "Received: from relay1.uu.net (HELO uunet.uu.net) (7@192.48.96.5)\n"
  "    by silverton.berkeley.edu with UTF8SMTP; 29 Nov 2020 04:46:54 -0000\n",
  /*- with folded recvd header and extra space before UTF8SMTP keyword */
  "Received: from relay1.uu.net (HELO uunet.uu.net) (7@192.48.96.5)\n"
  "    by silverton.berkeley.edu with   UTF8SMTP; 29 Nov 2020 04:46:54 -0000\n",
  0
};

char *ehlo_replies[] = {
  /*- SMTP Greeting */
  "220 indimail.org (NO UCE) ESMTP IndiMail 1.232 Sun, 6 Dec 2020 11:32:16 +0530\r\n",
  /*- EHLO Response */
  "250-indimail.org [::ffff:127.0.0.1]\r\n"
  "250-AUTH LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA256 CRAM-SHA512 CRAM-RIPEMD DIGEST-MD5\r\n"
  "250-PIPELINING\r\n"
  "250-8BITMIME\r\n"
  "250-SIZE 10000000\r\n"
  "250-ETRN\r\n"
  "250-STARTTLS\r\n"
  "250-SMTPUTF8\r\n"
  "250 HELP\r\n",
  /*- Deliberate Wrong Data */
  "gibberrish code\r\n",
  /*- EHLO response with differing codes */
  "250-indimail.org [::ffff:127.0.0.1]\r\n"
  "250-PIPELINING\r\n"
  "280-8BITMIME\r\n"
  "250-SMTPUTF8\r\n"
  "250 HELP\r\n",
  /*- temporary error */
  "420 Server unavailable\r\n",
  0
};

char *mailfrom_str[] = {
  "FROM:<abcd@notqmail.org> SIZE=2233 SMTPUTF8",
  "FROM:<abcd@notqmail.org> SMTPUTF8",
  "FROM:<abcd@notqmail.org> AUTH=abcd@notqmail.org SIZE=3434 SMTPUTF8",
  "FROM:<abcd@notqmail.org> AUTH=abcd@notqmail.org SMTPUTF8 SIZE=23323",
  "FROM:<abcd@notqmail.org> SMTPUTF8 AUTH=abcd@notqmail.org SIZE=23323",
  "FROM:<abcd@notqmail.org> AUTH=abcd@notqmail.org SIZE=23323",
};

START_TEST(test_utf8read1)
{
  int fd, i, ret;

  /*- non-utf mail */
  fd = open("mail.txt", O_RDWR|O_TRUNC|O_CREAT,0644);
  ck_assert_int_ne(fd, -1);
  ret = write(fd, maildata_1, (i = str_len(maildata_1)));
  ck_assert_int_eq(ret, i);
  ret = write(fd, maildata_2, (i = str_len(maildata_2)));
  ck_assert_int_eq(ret, i);
  ret = seek_set(fd, 0);
  ck_assert_int_eq(ret, 0);
  ret = dup2(fd, 0);
  ck_assert_int_ne(ret, -1);
#ifdef SMTPUTF8
  ret = utf8read(); /*- we should return 0 */
#else
  ret = -1;
#endif
  ck_assert_int_eq(ret, 0);
  close(fd);
  unlink("mail.txt");
}
END_TEST

START_TEST(test_utf8read2)
{
  int fd, i, j, ret;

  for (j=0;j < 3;j++) {
    /*- -utf mail2 */
    flagutf8 = 0;
    fd = open("mail.txt", O_RDWR|O_TRUNC|O_CREAT,0644);
    ck_assert_int_ne(fd, -1);
    ret = write(fd, maildata_1, (i = str_len(maildata_1)));
    ck_assert_int_eq(ret, i);
    ret = write(fd, received[j], (i = str_len(received[j])));
    ck_assert_int_eq(ret, i);
    ret = seek_set(fd, 0);
    ck_assert_int_eq(ret, 0);
    dup2(fd, 0);
#ifdef SMTPUTF8
    ret = utf8read(); /*- we should return 1 */
#else
  ret = -1;
#endif
    ck_assert_int_eq(ret, 1);
    close(fd);
  }
  unlink("mail.txt");
}
END_TEST

START_TEST(test_utf8read3)
{
  int fd, i, ret;

  /*- non-utf mail */
  fd = open("mail.txt", O_RDWR|O_TRUNC|O_CREAT,0644);
  ck_assert_int_ne(fd, -1);
  ret = write(fd, maildata_1, (i = str_len(maildata_1)));
  ck_assert_int_eq(ret, i);
  ret = write(fd, maildata_2, (i = str_len(maildata_2)));
  ck_assert_int_eq(ret, i);
  ret = seek_set(fd, 0);
  ck_assert_int_eq(ret, 0);
  dup2(fd, 0);
  flagutf8=1;
#ifdef SMTPUTF8
  ret = utf8read(); /*- we should return 1 because flagutf8 is already set */
#else
  ret = -1;
#endif
  /*- we should return 1 because we have set the flagutf8 */
  ck_assert_int_eq(ret, 1);
  close(fd);
  unlink("mail.txt");
}
END_TEST

Suite *utf8read_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("qmail-remote utf8read");

    /* core test case */
    tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_utf8read1);
    tcase_add_test(tc_core, test_utf8read2);
    tcase_add_test(tc_core, test_utf8read3);
    suite_add_tcase(s, tc_core);

    return s;
}

START_TEST(test_get_capability)
{
  int ret;

  if (!stralloc_copys(&smtptext, ehlo_replies[1])
      || !stralloc_0(&smtptext))
    temp_nomem();
  ret = get_capability("SMTPUTF8");
  ck_assert_int_eq(ret, 1);
  ret = get_capability("STARTTLS");
  ck_assert_int_eq(ret, 1);
  ret = get_capability("ENHANCEDSTATUSCODES");
  ck_assert_int_eq(ret, 0);
  ret = get_capability("DSN");
  ck_assert_int_eq(ret, 0);
}
END_TEST

Suite *capability_suite(void)
{
    Suite *s;
    TCase *tc_capa;
    s = suite_create("qmail-remote capability");
    tc_capa = tcase_create("capa");
    tcase_add_test(tc_capa, test_get_capability);
    suite_add_tcase(s, tc_capa);
  return s;
}

/*- here we test 4 cases
 * 1: SMTP server greeting 220
 * 2: Test with some gibberish data.
 * 3: Test with a multiline output
 * 4: Test with a multiline output with differing codes in each line
 */
START_TEST(test_smtpcode1)
{
  int fd, i, ret;
  unsigned long code;

  /*- smtpcode test 1 */
  fd = open("smtpcodes.tmp", O_RDWR|O_TRUNC|O_CREAT,0644);
  ck_assert_int_ne(fd, -1);
  /*- write greeting */
  ret = write(fd, ehlo_replies[0], (i = str_len(ehlo_replies[0])));
  ck_assert_int_eq(ret, i);
  ret = seek_set(fd, 0);
  ck_assert_int_eq(ret, 0);
  dup2(fd, 0);
  code = smtpcode();
  ck_assert_int_eq(code, 220);
  close(fd);
  unlink("smtpcodes.tmp");
}
END_TEST

START_TEST(test_smtpcode2)
{
  int fd, i, ret;
  unsigned long code;
  /*- smtpcode test 2 */
  fd = open("smtpcodes.tmp", O_RDWR|O_TRUNC|O_CREAT,0644);
  ck_assert_int_ne(fd, -1);
  /*- test multiline EHLO response*/
  ret = write(fd, ehlo_replies[1], (i = str_len(ehlo_replies[1])));
  ck_assert_int_eq(ret, i);
  ret = seek_set(fd, 0);
  ck_assert_int_eq(ret, 0);
  dup2(fd, 0);
  code = smtpcode();
  ck_assert_int_eq(code, 250);
  close(fd);
  unlink("smtpcodes.tmp");
}
END_TEST

START_TEST(test_smtpcode3)
{
  int fd, i, ret;
  unsigned long code;
  /*- smtpcode test 3 */
  fd = open("smtpcodes.tmp", O_RDWR|O_TRUNC|O_CREAT,0644);
  ck_assert_int_ne(fd, -1);
  /*- write gibberish data so that smtpcode returns failure */
  ret = write(fd, ehlo_replies[2], (i = str_len(ehlo_replies[2])));
  ck_assert_int_eq(ret, i);
  ret = seek_set(fd, 0);
  ck_assert_int_eq(ret, 0);
  dup2(fd, 0);
  code = smtpcode();
  ck_assert_int_eq(code, 400);
  close(fd);
  unlink("smtpcodes.tmp");
}
END_TEST

START_TEST(test_smtpcode4)
{
  int fd, i, ret;
  unsigned long code;
  /*- smtpcode test 4 */
  fd = open("smtpcodes.tmp", O_RDWR|O_TRUNC|O_CREAT,0644);
  ck_assert_int_ne(fd, -1);
  /*- test multiline EHLO response*/
  ret = write(fd, ehlo_replies[3], (i = str_len(ehlo_replies[3])));
  ck_assert_int_eq(ret, i);
  ret = seek_set(fd, 0);
  ck_assert_int_eq(ret, 0);
  dup2(fd, 0);
  code = smtpcode();
  ck_assert_int_ne(code, 250);
  close(fd);
  unlink("smtpcodes.tmp");
}
END_TEST

Suite *smtpcode_suite(void)
{
  Suite *s;
  TCase *tc_smtpcode;
  s = suite_create("qmail-remote smtpcode");
  tc_smtpcode = tcase_create("smtpcode");
  tcase_add_test(tc_smtpcode, test_smtpcode1);
  tcase_add_test(tc_smtpcode, test_smtpcode2);
  tcase_add_test(tc_smtpcode, test_smtpcode3);
  tcase_add_test(tc_smtpcode, test_smtpcode4);
  suite_add_tcase(s, tc_smtpcode);
  return s;
}

START_TEST(test_mailfrom_parms)
{
  int i;

  for (i=0;i<5;i++) {
    smtputf8=0;
    mailfrom_parms(mailfrom_str[i]);
    ck_assert_int_eq(smtputf8, 1);
  }
  smtputf8=0;
  mailfrom_parms(mailfrom_str[5]);
  ck_assert_int_ne(smtputf8, 1);
}
END_TEST

Suite *mailfrom_parms_suite(void)
{
  Suite *s;
  TCase *tc_mailfrom_parms;
  s = suite_create("qmail-smtpd mailfrom_parms");
  tc_mailfrom_parms = tcase_create("mailfrom_parms");
  tcase_add_test(tc_mailfrom_parms, test_mailfrom_parms);
  suite_add_tcase(s, tc_mailfrom_parms);
  return s;
}

int main(void)
{
  int number_failed;
  Suite *s1, *s2, *s3, *s4;
  SRunner *sr1, *sr2, *sr3, *sr4;

  s1 = utf8read_suite();
  s2 = capability_suite();
  s3 = smtpcode_suite();
  s4 = mailfrom_parms_suite();
  sr1 = srunner_create(s1);
  sr2 = srunner_create(s2);
  sr3 = srunner_create(s3);
  sr4 = srunner_create(s4);

  srunner_run_all(sr1, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr1);

  srunner_run_all(sr2, CK_NORMAL);
  number_failed += srunner_ntests_failed(sr2);

  srunner_run_all(sr3, CK_NORMAL);
  number_failed += srunner_ntests_failed(sr3);

  srunner_run_all(sr4, CK_NORMAL);
  number_failed += srunner_ntests_failed(sr4);

  srunner_free(sr1);
  srunner_free(sr2);
  srunner_free(sr3);
  srunner_free(sr4);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
