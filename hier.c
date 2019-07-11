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

  d(auto_qmail_queue,base,uid,auto_gidq,mode);

  for (i = 0;i < auto_split;++i) {
    x = buf;
    x += fmt_str(x,base);
    x += fmt_str(x,"/");
    x += fmt_ulong(x,i);
    *x = 0;

    d(auto_qmail_queue,buf,uid,auto_gidq,mode);
  }
}

void hier()
{
  h(auto_qmail,auto_uido,auto_gidq,0755);
  h(auto_qmail_bin,auto_uido,auto_gidq,0755);
  h(auto_qmail_boot,auto_uido,auto_gidq,0755);
  h(auto_qmail_doc,auto_uido,auto_gidq,0755);
  h(auto_qmail_man,auto_uido,auto_gidq,0755);

  h(auto_qmail_control,auto_uido,auto_gidq,0755);
  h(auto_qmail_users,auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"cat1",auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"cat5",auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"cat7",auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"cat8",auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"man1",auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"man5",auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"man7",auto_uido,auto_gidq,0755);
  d(auto_qmail_man,"man8",auto_uido,auto_gidq,0755);

  h(auto_qmail_alias,auto_uida,auto_gidq,02755);

  h(auto_qmail_queue,auto_uidq,auto_gidq,0750);
  d(auto_qmail_queue,"pid",auto_uidq,auto_gidq,0700);
  d(auto_qmail_queue,"intd",auto_uidq,auto_gidq,0700);
  d(auto_qmail_queue,"todo",auto_uidq,auto_gidq,0750);
  d(auto_qmail_queue,"bounce",auto_uids,auto_gidq,0700);

  dsplit("mess",auto_uidq,0750);
  dsplit("info",auto_uids,0700);
  dsplit("local",auto_uids,0700);
  dsplit("remote",auto_uids,0700);

  d(auto_qmail_queue,"lock",auto_uidq,auto_gidq,0750);
  z(auto_qmail_queue,"lock/tcpto",1024,auto_uidr,auto_gidq,0644);
  z(auto_qmail_queue,"lock/sendmutex",0,auto_uids,auto_gidq,0600);
  p(auto_qmail_queue,"lock/trigger",auto_uids,auto_gidq,0622);

  c(auto_qmail_boot,".","home",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","home+df",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","proc",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","proc+df",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","binm1",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","binm1+df",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","binm2",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","binm2+df",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","binm3",auto_uido,auto_gidq,0755);
  c(auto_qmail_boot,".","binm3+df",auto_uido,auto_gidq,0755);

  c(auto_qmail_doc,".","FAQ",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","UPGRADE",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","SENDMAIL.md",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","INSTALL.md",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","INSTALL.alias",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","INSTALL.ctl",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","INSTALL.ids",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","INSTALL.maildir",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","INSTALL.mbox",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","INSTALL.vsm",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","TEST.deliver",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","TEST.receive",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","REMOVE.sendmail",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","REMOVE.binmail",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.local2alias",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.local2ext",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.local2local",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.local2rem",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.local2virt",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.nullclient",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.relaybad",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.relaygood",auto_uido,auto_gidq,0644);
  c(auto_qmail_doc,".","PIC.rem2local",auto_uido,auto_gidq,0644);

  c(auto_qmail_bin,".","qmail-queue",auto_uidq,auto_gidq,04711);
  c(auto_qmail_bin,".","qmail-lspawn",auto_uido,auto_gidq,0700);
  c(auto_qmail_bin,".","qmail-start",auto_uido,auto_gidq,0700);
  c(auto_qmail_bin,".","qmail-getpw",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-local",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-remote",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-rspawn",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-clean",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-send",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","splogger",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-newu",auto_uido,auto_gidq,0700);
  c(auto_qmail_bin,".","qmail-newmrh",auto_uido,auto_gidq,0700);
  c(auto_qmail_bin,".","qmail-pw2u",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-inject",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","predate",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","datemail",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","mailsubj",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-showctl",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-qread",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-qstat",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-tcpto",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-tcpok",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-pop3d",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-popup",auto_uido,auto_gidq,0711);
  c(auto_qmail_bin,".","qmail-qmqpc",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-qmqpd",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-qmtpd",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qmail-smtpd",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","sendmail",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","tcp-env",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qreceipt",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qsmhook",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qbiff",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","forward",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","preline",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","condredirect",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","bouncesaying",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","except",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","maildirmake",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","maildir2mbox",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","maildirwatch",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","qail",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","elq",auto_uido,auto_gidq,0755);
  c(auto_qmail_bin,".","pinq",auto_uido,auto_gidq,0755);

  c(auto_qmail_man,"man5","addresses.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","addresses.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","envelopes.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","envelopes.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","maildir.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","maildir.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","mbox.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","mbox.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","dot-qmail.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","dot-qmail.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","qmail-control.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","qmail-control.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","qmail-header.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","qmail-header.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","qmail-log.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","qmail-log.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","qmail-users.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","qmail-users.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man5","tcp-environ.5",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat5","tcp-environ.0",auto_uido,auto_gidq,0644);

  c(auto_qmail_man,"man7","forgeries.7",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat7","forgeries.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man7","qmail-limits.7",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat7","qmail-limits.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man7","qmail.7",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat7","qmail.0",auto_uido,auto_gidq,0644);

  c(auto_qmail_man,"man1","forward.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","forward.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","condredirect.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","condredirect.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","bouncesaying.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","bouncesaying.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","except.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","except.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","maildirmake.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","maildirmake.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","maildir2mbox.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","maildir2mbox.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","maildirwatch.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","maildirwatch.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","mailsubj.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","mailsubj.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","qreceipt.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","qreceipt.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","qbiff.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","qbiff.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","preline.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","preline.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man1","tcp-env.1",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat1","tcp-env.0",auto_uido,auto_gidq,0644);

  c(auto_qmail_man,"man8","qmail-local.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-local.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-lspawn.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-lspawn.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-getpw.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-getpw.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-remote.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-remote.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-rspawn.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-rspawn.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-clean.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-clean.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-send.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-send.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-start.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-start.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","splogger.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","splogger.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-queue.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-queue.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-inject.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-inject.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-showctl.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-showctl.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-newmrh.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-newmrh.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-newu.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-newu.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-pw2u.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-pw2u.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-qread.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-qread.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-qstat.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-qstat.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-tcpok.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-tcpok.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-tcpto.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-tcpto.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-pop3d.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-pop3d.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-popup.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-popup.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-qmqpc.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-qmqpc.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-qmqpd.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-qmqpd.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-qmtpd.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-qmtpd.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-smtpd.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-smtpd.0",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"man8","qmail-command.8",auto_uido,auto_gidq,0644);
  c(auto_qmail_man,"cat8","qmail-command.0",auto_uido,auto_gidq,0644);
}
