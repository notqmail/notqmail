#include "qmail.h"

#include "substdio.h"
#include "readwrite.h"
#include "wait.h"
#include "exit.h"
#include "fork.h"
#include "fd.h"
#include "auto_qmail.h"
#include "env.h"

static char *binqqargs[2] = { 0, 0 } ;

static void setup_qqargs()
{
  if(!binqqargs[0])
    binqqargs[0] = env_get("QMAILQUEUE");
  if(!binqqargs[0])
    binqqargs[0] = "bin/qmail-queue";
}

int qmail_open(qq)
struct qmail *qq;
{
  int pim[2];
  int pie[2];
  int pierr[2];

  setup_qqargs();

  if (pipe(pim) == -1) return -1;
  if (pipe(pie) == -1) { close(pim[0]); close(pim[1]); return -1; }
  if (pipe(pierr) == -1) {
    close(pim[0]); close(pim[1]);
    close(pie[0]); close(pie[1]);
    return -1;
  }
 
  switch(qq->pid = fork()) {
    case -1:
      close(pim[0]); close(pim[1]);
      close(pie[0]); close(pie[1]);
      close(pierr[0]); close(pierr[1]);
      return -1;
    case 0:
      close(pim[1]);
      close(pie[1]);
      close(pierr[0]); /* we want to receive data */
      if (fd_move(0,pim[0]) == -1) _exit(120);
      if (fd_move(1,pie[0]) == -1) _exit(120);
      if (fd_move(6,pierr[1]) == -1) _exit(120);
      if (chdir(auto_qmail) == -1) _exit(61);
      execv(*binqqargs,binqqargs);
      _exit(120);
  }

  qq->fdm = pim[1]; close(pim[0]);
  qq->fde = pie[1]; close(pie[0]);
  qq->fderr = pierr[0]; close(pierr[1]);
  substdio_fdbuf(&qq->ss,write,qq->fdm,qq->buf,sizeof(qq->buf));
  qq->flagerr = 0;
  return 0;
}

unsigned long qmail_qp(qq) struct qmail *qq;
{
  return qq->pid;
}

void qmail_fail(qq) struct qmail *qq;
{
  qq->flagerr = 1;
}

void qmail_put(struct qmail *qq, char *s, size_t len)
{
  if (!qq->flagerr) if (substdio_put(&qq->ss,s,len) == -1) qq->flagerr = 1;
}

void qmail_from(qq,s) struct qmail *qq; char *s;
{
  if (substdio_flush(&qq->ss) == -1) qq->flagerr = 1;
  close(qq->fdm);
  substdio_fdbuf(&qq->ss,write,qq->fde,qq->buf,sizeof(qq->buf));
  qmail_put(qq,"F",1);
  qmail_puts(qq,s);
  qmail_put(qq,"",1);
}

void qmail_to(qq,s) struct qmail *qq; char *s;
{
  qmail_put(qq,"T",1);
  qmail_puts(qq,s);
  qmail_put(qq,"",1);
}

static size_t qmail_errstr(struct qmail *qq, char *s) {
  size_t len = 0;
  substdio_fdbuf(&qq->ss,read,qq->fderr,qq->buf,sizeof(qq->buf));
  while (substdio_get(&qq->ss,s+len,1) > 0 && len < 255) {
    len++;
  }
  s[len] = '\0';
  return len;
}

char *qmail_close(qq)
struct qmail *qq;
{
  int wstat;
  int exitcode;
  static char errstr[256];
  size_t errlen;

  qmail_put(qq,"",1);
  if (!qq->flagerr) if (substdio_flush(&qq->ss) == -1) qq->flagerr = 1;
  close(qq->fde);
  errlen = qmail_errstr(qq, errstr);
  close(qq->fderr);

  if (wait_pid(&wstat,qq->pid) != qq->pid)
    return "Zqq waitpid surprise (#4.3.0)";
  if (wait_crashed(wstat))
    return "Zqq crashed (#4.3.0)";
  exitcode = wait_exitcode(wstat);

  switch(exitcode) {
    case 115: /* compatibility */
    case 11: return "Denvelope address too long for qq (#5.1.3)";
    case 31: return "Dmail server permanently rejected message (#5.3.0)";
    case 51: return "Zqq out of memory (#4.3.0)";
    case 52: return "Zqq timeout (#4.3.0)";
    case 53: return "Zqq write error or disk full (#4.3.0)";
    case 0: if (!qq->flagerr) return ""; /* fall through */
    case 54: return "Zqq read error (#4.3.0)";
    case 55: return "Zqq unable to read configuration (#4.3.0)";
    case 56: return "Zqq trouble making network connection (#4.3.0)";
    case 61: return "Zqq trouble in home directory (#4.3.0)";
    case 63:
    case 64:
    case 65:
    case 66:
    case 62: return "Zqq trouble creating files in queue (#4.3.0)";
    case 71: return "Zmail server temporarily rejected message (#4.3.0)";
    case 72: return "Zconnection to mail server timed out (#4.4.1)";
    case 73: return "Zconnection to mail server rejected (#4.4.1)";
    case 74: return "Zcommunication with mail server failed (#4.4.2)";
    case 91: /* fall through */
    case 81: return "Zqq internal bug (#4.3.0)";
    case 120: return "Zunable to exec qq (#4.3.0)";
    default:
      if (exitcode == 82 && errlen > 2)
        return errstr;
      if ((exitcode >= 11) && (exitcode <= 40))
	return "Dqq permanent problem (#5.3.0)";
      return "Zqq temporary problem (#4.3.0)";
  }
}
