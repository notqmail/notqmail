#include <unistd.h>
#include "control.h"
#include "env.h"
#include "stralloc.h"
#include "substdio.h"
#include "wait.h"

static char outbuf[128];
static struct substdio ssout = SUBSTDIO_FDBUF(write,1,outbuf,sizeof outbuf);
static char errbuf[128];
static struct substdio sserr = SUBSTDIO_FDBUF(write,2,errbuf,sizeof errbuf);

struct rcptcheck_api {
  char *(*get_sender)();
  char *(*get_recipient)();
  void (*accept_recipient)();
  void (*reject_recipient)();
  void (*unable_to_verify)();
  void (*unable_to_execute)();
} rcptcheck;

static char *rcptcheck_get_sender() {
  return env_get("SENDER");
}

static char *rcptcheck_get_recipient() {
  return env_get("RECIPIENT");
}

static void rcptcheck_accept_recipient() {
  _exit(0);
}

static void rcptcheck_reject_recipient() {
  _exit(100);
}

static void rcptcheck_unable_to_verify() {
  _exit(111);
}

static void rcptcheck_unable_to_execute() {
  _exit(120);
}

static void run_rcptchecks_under_rcptcheck() {
  rcptcheck.get_sender        = rcptcheck_get_sender;
  rcptcheck.get_recipient     = rcptcheck_get_recipient;
  rcptcheck.accept_recipient  = rcptcheck_accept_recipient;
  rcptcheck.reject_recipient  = rcptcheck_reject_recipient;
  rcptcheck.unable_to_verify  = rcptcheck_unable_to_verify;
  rcptcheck.unable_to_execute = rcptcheck_unable_to_execute;
}

static void putsdie(substdio *ss,char *string) {
  substdio_puts(ss,string);
  substdio_flush(ss);
  _exit(0);
}

static char *spp_get_sender() {
  return env_get("SMTPMAILFROM");
}

static char *spp_get_recipient() {
  return env_get("SMTPRCPTTO");
}

static void spp_accept_recipient() {
  putsdie(&ssout,"");
}

static void spp_reject_recipient() {
  putsdie(&ssout,"E553 sorry, no mailbox here by that name. (#5.1.1)\n");
}

static void spp_unable_to_verify() {
  putsdie(&ssout,"R421 unable to verify recipient (#4.3.0)\n");
}

static void spp_unable_to_execute() {
  putsdie(&ssout,"R421 unable to execute recipient check (#4.3.0)\n");
}

static void run_rcptchecks_under_spp() {
  rcptcheck.get_sender        = spp_get_sender;
  rcptcheck.get_recipient     = spp_get_recipient;
  rcptcheck.accept_recipient  = spp_accept_recipient;
  rcptcheck.reject_recipient  = spp_reject_recipient;
  rcptcheck.unable_to_verify  = spp_unable_to_verify;
  rcptcheck.unable_to_execute = spp_unable_to_execute;
}

static void run_rcptcheck(char *program)
{
  char *rcptcheck_program[2] = { program, 0 };
  int pid;
  int wstat;

  switch(pid = fork()) {
    case -1:
      rcptcheck.unable_to_execute();
    case 0:
      execv(*rcptcheck_program,rcptcheck_program);
      rcptcheck.unable_to_execute();
  }

  if (wait_pid(&wstat,pid) == -1)
    rcptcheck.unable_to_execute();
  if (wait_crashed(wstat))
    rcptcheck.unable_to_execute();

  switch(wait_exitcode(wstat)) {
    case 100: rcptcheck.reject_recipient();
    case 111: rcptcheck.unable_to_verify();
    case 120: rcptcheck.unable_to_execute();
    default:  return;
  }
}

static int looks_like_spp() {
  char *x = env_get("SMTPRCPTTO");
  return (x != 0);
}

static void set_environment(char *sender, char *recipient) {
  substdio_puts(&sserr,"qmail-rcptcheck: from ");

  if (sender) {
    substdio_puts(&sserr,sender);
    if (!env_put2("SENDER", sender)) rcptcheck.unable_to_verify();
  }

  substdio_puts(&sserr," to ");

  if (recipient) {
    substdio_puts(&sserr,recipient);
    if (!env_put2("RECIPIENT", recipient)) rcptcheck.unable_to_verify();
  }

  substdio_puts(&sserr,"\n");
  substdio_flush(&sserr);
}

int main(void)
{
  stralloc rcptchecks = {0};
  int linestart;
  int pos;

  if (looks_like_spp()) run_rcptchecks_under_spp();
  else run_rcptchecks_under_rcptcheck();

  set_environment(rcptcheck.get_sender(), rcptcheck.get_recipient());

  if (control_readfile(&rcptchecks,"control/rcptchecks",0) == -1)
    rcptcheck.unable_to_verify();

  for (linestart = 0, pos = 0; pos < rcptchecks.len; pos++) {
    if (rcptchecks.s[pos] == '\0') {
      run_rcptcheck(rcptchecks.s + linestart);
      linestart = pos + 1;
    }
  }

  rcptcheck.accept_recipient();
}
