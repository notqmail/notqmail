#include "sgetopt.h"
#include "substdio.h"
#include "subfd.h"
#include "alloc.h"
#include "auto_qmail.h"
#include "exit.h"
#include "env.h"
#include "str.h"

void nomem()
{
  substdio_putsflush(subfderr,"sendmail: fatal: out of memory\n");
  _exit(111);
}

void die_usage()
{
  substdio_putsflush(subfderr,"sendmail: usage: sendmail [ -t ] [ -fsender ] [ -Fname ] [ -bp ] [ -bs ] [ arg ... ]\n");
  _exit(100);
}

char *smtpdarg[] = { "bin/qmail-smtpd", 0 };
void smtpd()
{
  if (!env_get("PROTO")) {
    if (!env_put("RELAYCLIENT=")) nomem();
    if (!env_put("DATABYTES=0")) nomem();
    if (!env_put("PROTO=TCP")) nomem();
    if (!env_put("TCPLOCALIP=127.0.0.1")) nomem();
    if (!env_put("TCPLOCALHOST=localhost")) nomem();
    if (!env_put("TCPREMOTEIP=127.0.0.1")) nomem();
    if (!env_put("TCPREMOTEHOST=localhost")) nomem();
    if (!env_put("TCPREMOTEINFO=sendmail-bs")) nomem();
  }
  execv(*smtpdarg,smtpdarg);
  substdio_putsflush(subfderr,"sendmail: fatal: unable to run qmail-smtpd\n");
  _exit(111);
}

char *qreadarg[] = { "bin/qmail-qread", 0 };
void mailq()
{
  execv(*qreadarg,qreadarg);
  substdio_putsflush(subfderr,"sendmail: fatal: unable to run qmail-qread\n");
  _exit(111);
}

int flagh;
char *sender;

void main(argc,argv)
int argc;
char **argv;
{
  int opt;
  char **qiargv;
  char **arg;
  int i;
 
  if (chdir(auto_qmail) == -1) {
    substdio_putsflush(subfderr,"sendmail: fatal: unable to switch to qmail home directory\n");
    _exit(111);
  }

  flagh = 0;
  sender = 0;
  while ((opt = getopt(argc,argv,"vimte:f:p:o:B:F:EJxb:")) != opteof)
    switch(opt) {
      case 'B': break;
      case 't': flagh = 1; break;
      case 'f': sender = optarg; break;
      case 'F': if (!env_put2("MAILNAME",optarg)) nomem(); break;
      case 'p': break; /* could generate a Received line from optarg */
      case 'v': break;
      case 'i': break; /* what an absurd concept */
      case 'x': break; /* SVR4 stupidity */
      case 'm': break; /* twisted-paper-path blindness, incompetent design */
      case 'e': break; /* qmail has only one error mode */
      case 'o':
        switch(optarg[0]) {
	  case 'd': break; /* qmail has only one delivery mode */
	  case 'e': break; /* see 'e' above */
	  case 'i': break; /* see 'i' above */
	  case 'm': break; /* see 'm' above */
	}
        break;
      case 'E': case 'J': /* Sony NEWS-OS */
        while (argv[optind][optpos]) ++optpos; /* skip optional argument */
        break;
      case 'b':
	switch(optarg[0]) {
	  case 'm': break;
	  case 'p': mailq();
	  case 's': smtpd();
	  default: die_usage();
	}
	break;
      default:
	die_usage();
    }
  argc -= optind;
  argv += optind;
 
  if (str_equal(optprogname,"mailq"))
    mailq();

  if (str_equal(optprogname,"newaliases")) {
    substdio_putsflush(subfderr,"sendmail: fatal: please use fastforward/newaliases instead\n");
    _exit(100);
  }

  qiargv = (char **) alloc((argc + 10) * sizeof(char *));
  if (!qiargv) nomem();
 
  arg = qiargv;
  *arg++ = "bin/qmail-inject";
  *arg++ = (flagh ? "-H" : "-a");
  if (sender) {
    *arg++ = "-f";
    *arg++ = sender;
  }
  *arg++ = "--";
  for (i = 0;i < argc;++i) *arg++ = argv[i];
  *arg = 0;
 
  execv(*qiargv,qiargv);
  substdio_putsflush(subfderr,"sendmail: fatal: unable to run qmail-inject\n");
  _exit(111);
}
