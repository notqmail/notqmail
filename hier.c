#include "auto_qmail.h"
#include "auto_split.h"
#include "auto_uids.h"
#include "fmt.h"
#include "fifo.h"

char buf[100 + FMT_ULONG];

void dsplit(base,uid,mode)
char *base; /* must be under 100 bytes */
int uid;
int mode;
{
  char *x;
  unsigned long i;

  d(auto_qmail,base,uid,auto_gidq,mode);

  for (i = 0;i < auto_split;++i) {
    x = buf;
    x += fmt_str(x,base);
    x += fmt_str(x,"/");
    x += fmt_ulong(x,i);
    *x = 0;

    d(auto_qmail,buf,uid,auto_gidq,mode);
  }
}

void hier()
{
  h(auto_qmail,auto_uido,auto_gidq,0755);

  d(auto_qmail,"control",auto_uido,auto_gidq,0755);
  d(auto_qmail,"users",auto_uido,auto_gidq,0755);
  d(auto_qmail,"bin",auto_uido,auto_gidq,0755);
  d(auto_qmail,"boot",auto_uido,auto_gidq,0755);
  d(auto_qmail,"doc",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/cat1",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/cat5",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/cat7",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/cat8",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/man1",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/man5",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/man7",auto_uido,auto_gidq,0755);
  d(auto_qmail,"man/man8",auto_uido,auto_gidq,0755);

  d(auto_qmail,"alias",auto_uida,auto_gidq,02755);

  d(auto_qmail,"queue",auto_uidq,auto_gidq,0750);
  d(auto_qmail,"queue/pid",auto_uidq,auto_gidq,0700);
  d(auto_qmail,"queue/intd",auto_uidq,auto_gidq,0700);
  d(auto_qmail,"queue/todo",auto_uidq,auto_gidq,0750);
  d(auto_qmail,"queue/bounce",auto_uids,auto_gidq,0700);

  dsplit("queue/mess",auto_uidq,0750);
  dsplit("queue/info",auto_uids,0700);
  dsplit("queue/local",auto_uids,0700);
  dsplit("queue/remote",auto_uids,0700);

  d(auto_qmail,"queue/lock",auto_uidq,auto_gidq,0750);
  z(auto_qmail,"queue/lock/tcpto",1024,auto_uidr,auto_gidq,0644);
  z(auto_qmail,"queue/lock/sendmutex",0,auto_uids,auto_gidq,0600);
  p(auto_qmail,"queue/lock/trigger",auto_uids,auto_gidq,0622);

  c(auto_qmail,"boot","home",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","home+df",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","proc",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","proc+df",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","binm1",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","binm1+df",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","binm2",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","binm2+df",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","binm3",auto_uido,auto_gidq,0755);
  c(auto_qmail,"boot","binm3+df",auto_uido,auto_gidq,0755);

  c(auto_qmail,"doc","FAQ",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","UPGRADE",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","SENDMAIL",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","INSTALL",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","INSTALL.alias",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","INSTALL.ctl",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","INSTALL.ids",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","INSTALL.maildir",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","INSTALL.mbox",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","INSTALL.vsm",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","TEST.deliver",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","TEST.receive",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","REMOVE.sendmail",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","REMOVE.binmail",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.local2alias",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.local2ext",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.local2local",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.local2rem",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.local2virt",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.nullclient",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.relaybad",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.relaygood",auto_uido,auto_gidq,0644);
  c(auto_qmail,"doc","PIC.rem2local",auto_uido,auto_gidq,0644);

  c(auto_qmail,"bin","qmail-queue",auto_uidq,auto_gidq,04711);
  c(auto_qmail,"bin","qmail-lspawn",auto_uido,auto_gidq,0700);
  c(auto_qmail,"bin","qmail-start",auto_uido,auto_gidq,0700);
  c(auto_qmail,"bin","qmail-getpw",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-local",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-remote",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-rspawn",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-clean",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-send",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","splogger",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-newu",auto_uido,auto_gidq,0700);
  c(auto_qmail,"bin","qmail-newmrh",auto_uido,auto_gidq,0700);
  c(auto_qmail,"bin","qmail-pw2u",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-inject",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","predate",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","datemail",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","mailsubj",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-showctl",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-qread",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-qstat",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-tcpto",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-tcpok",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-pop3d",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-popup",auto_uido,auto_gidq,0711);
  c(auto_qmail,"bin","qmail-qmqpc",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-qmqpd",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-qmtpd",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qmail-smtpd",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","sendmail",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","tcp-env",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qreceipt",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qsmhook",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qbiff",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","forward",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","preline",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","condredirect",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","bouncesaying",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","except",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","maildirmake",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","maildir2mbox",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","maildirwatch",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","qail",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","elq",auto_uido,auto_gidq,0755);
  c(auto_qmail,"bin","pinq",auto_uido,auto_gidq,0755);

  c(auto_qmail,"man/man5","addresses.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","addresses.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","envelopes.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","envelopes.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","maildir.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","maildir.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","mbox.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","mbox.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","dot-qmail.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","dot-qmail.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","qmail-control.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","qmail-control.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","qmail-header.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","qmail-header.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","qmail-log.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","qmail-log.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","qmail-users.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","qmail-users.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man5","tcp-environ.5",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat5","tcp-environ.0",auto_uido,auto_gidq,0644);

  c(auto_qmail,"man/man7","forgeries.7",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat7","forgeries.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man7","qmail-limits.7",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat7","qmail-limits.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man7","qmail.7",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat7","qmail.0",auto_uido,auto_gidq,0644);

  c(auto_qmail,"man/man1","forward.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","forward.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","condredirect.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","condredirect.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","bouncesaying.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","bouncesaying.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","except.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","except.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","maildirmake.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","maildirmake.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","maildir2mbox.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","maildir2mbox.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","maildirwatch.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","maildirwatch.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","mailsubj.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","mailsubj.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","qreceipt.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","qreceipt.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","qbiff.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","qbiff.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","preline.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","preline.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man1","tcp-env.1",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat1","tcp-env.0",auto_uido,auto_gidq,0644);

  c(auto_qmail,"man/man8","qmail-local.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-local.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-lspawn.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-lspawn.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-getpw.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-getpw.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-remote.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-remote.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-rspawn.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-rspawn.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-clean.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-clean.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-send.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-send.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-start.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-start.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","splogger.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","splogger.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-queue.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-queue.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-inject.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-inject.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-showctl.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-showctl.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-newmrh.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-newmrh.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-newu.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-newu.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-pw2u.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-pw2u.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-qread.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-qread.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-qstat.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-qstat.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-tcpok.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-tcpok.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-tcpto.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-tcpto.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-pop3d.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-pop3d.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-popup.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-popup.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-qmqpc.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-qmqpc.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-qmqpd.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-qmqpd.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-qmtpd.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-qmtpd.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-smtpd.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-smtpd.0",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/man8","qmail-command.8",auto_uido,auto_gidq,0644);
  c(auto_qmail,"man/cat8","qmail-command.0",auto_uido,auto_gidq,0644);
}
