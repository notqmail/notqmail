#include "subfd.h"
#include "substdio.h"
#include "auto_split.h"
#include "auto_uids.h"
#include "fmt.h"

char strnum[FMT_ULONG];

void uidgid(uid)
int uid;
{
  substdio_put(subfdout,strnum,fmt_ulong(strnum,(unsigned long) uid));
  substdio_puts(subfdout,":");
  substdio_put(subfdout,strnum,fmt_ulong(strnum,(unsigned long) auto_gidq));
  substdio_puts(subfdout,":");
}

void copy(uid,mode,sub,fn)
int uid;
char *mode;
char *sub;
char *fn;
{
  substdio_puts(subfdout,"c:");
  uidgid(uid);
  substdio_puts(subfdout,mode);
  substdio_puts(subfdout,":");
  substdio_puts(subfdout,sub);
  substdio_puts(subfdout,":");
  substdio_puts(subfdout,fn);
  substdio_puts(subfdout,":\n");
}

void dir(uid,mode,fn)
int uid;
char *mode;
char *fn;
{
  substdio_puts(subfdout,"d:");
  uidgid(uid);
  substdio_puts(subfdout,mode);
  substdio_puts(subfdout,":");
  substdio_puts(subfdout,fn);
  substdio_puts(subfdout,"::\n");
}

void dirsplit(uid,mode,fn)
int uid;
char *mode;
char *fn;
{
  unsigned long i;
  dir(uid,mode,fn);
  for (i = 0;i < auto_split;++i) {
    substdio_puts(subfdout,"d:");
    uidgid(uid);
    substdio_puts(subfdout,mode);
    substdio_puts(subfdout,":");
    substdio_puts(subfdout,fn);
    substdio_puts(subfdout,":/");
    substdio_put(subfdout,strnum,fmt_ulong(strnum,i));
    substdio_puts(subfdout,":\n");
  }
}

void main()
{
  dir(auto_uido,"755","");
  dir(auto_uido,"755","/control");
  dir(auto_uido,"755","/users");
  dir(auto_uido,"755","/bin");
  dir(auto_uido,"755","/man");
  dir(auto_uido,"755","/man/cat1");
  dir(auto_uido,"755","/man/cat5");
  dir(auto_uido,"755","/man/cat7");
  dir(auto_uido,"755","/man/cat8");
  dir(auto_uido,"755","/man/man1");
  dir(auto_uido,"755","/man/man5");
  dir(auto_uido,"755","/man/man7");
  dir(auto_uido,"755","/man/man8");
  dir(auto_uido,"755","/doc");
  dir(auto_uido,"755","/boot");

  copy(auto_uido,"755","/boot/","home");
  copy(auto_uido,"755","/boot/","home+df");
  copy(auto_uido,"755","/boot/","proc");
  copy(auto_uido,"755","/boot/","proc+df");
  copy(auto_uido,"755","/boot/","binm1");
  copy(auto_uido,"755","/boot/","binm1+df");
  copy(auto_uido,"755","/boot/","binm2");
  copy(auto_uido,"755","/boot/","binm2+df");
  copy(auto_uido,"755","/boot/","binm3");
  copy(auto_uido,"755","/boot/","binm3+df");

  copy(auto_uido,"644","/doc/","FAQ");
  copy(auto_uido,"644","/doc/","UPGRADE");
  copy(auto_uido,"644","/doc/","SENDMAIL");
  copy(auto_uido,"644","/doc/","INSTALL");
  copy(auto_uido,"644","/doc/","INSTALL.alias");
  copy(auto_uido,"644","/doc/","INSTALL.boot");
  copy(auto_uido,"644","/doc/","INSTALL.ctl");
  copy(auto_uido,"644","/doc/","INSTALL.ids");
  copy(auto_uido,"644","/doc/","INSTALL.maildir");
  copy(auto_uido,"644","/doc/","INSTALL.mbox");
  copy(auto_uido,"644","/doc/","INSTALL.vsm");
  copy(auto_uido,"644","/doc/","PIC.local2alias");
  copy(auto_uido,"644","/doc/","PIC.local2ext");
  copy(auto_uido,"644","/doc/","PIC.local2local");
  copy(auto_uido,"644","/doc/","PIC.local2rem");
  copy(auto_uido,"644","/doc/","PIC.local2virt");
  copy(auto_uido,"644","/doc/","PIC.nullclient");
  copy(auto_uido,"644","/doc/","PIC.relaybad");
  copy(auto_uido,"644","/doc/","PIC.relaygood");
  copy(auto_uido,"644","/doc/","PIC.rem2local");

  dir(auto_uida,"2755","/alias");
  dir(auto_uidq,"750","/queue");
  dir(auto_uidq,"700","/queue/pid");
  dir(auto_uidq,"700","/queue/intd");
  dir(auto_uidq,"750","/queue/todo");
  dir(auto_uidq,"750","/queue/lock");
  dir(auto_uids,"700","/queue/bounce");

  substdio_puts(subfdout,"z0:");
  uidgid(auto_uids);
  substdio_puts(subfdout,"600:/queue/lock/:sendmutex:\n");

  substdio_puts(subfdout,"z1024:");
  uidgid(auto_uidr);
  substdio_puts(subfdout,"644:/queue/lock/:tcpto:\n");

  substdio_puts(subfdout,"p:");
  uidgid(auto_uids);
  substdio_puts(subfdout,"622:/queue/lock/:trigger:\n");

  dirsplit(auto_uidq,"750","/queue/mess");
  dirsplit(auto_uids,"700","/queue/info");
  dirsplit(auto_uids,"700","/queue/local");
  dirsplit(auto_uids,"700","/queue/remote");

  copy(auto_uidq,"4711","/bin/","qmail-queue");
  copy(auto_uido,"700","/bin/","qmail-lspawn");
  copy(auto_uido,"700","/bin/","qmail-start");
  copy(auto_uido,"711","/bin/","qmail-getpw");
  copy(auto_uido,"711","/bin/","qmail-local");
  copy(auto_uido,"711","/bin/","qmail-remote");
  copy(auto_uido,"711","/bin/","qmail-rspawn");
  copy(auto_uido,"711","/bin/","qmail-clean");
  copy(auto_uido,"711","/bin/","qmail-send");
  copy(auto_uido,"711","/bin/","splogger");
  copy(auto_uido,"700","/bin/","qmail-newu");
  copy(auto_uido,"700","/bin/","qmail-newmrh");
  copy(auto_uido,"711","/bin/","qmail-pw2u");
  copy(auto_uido,"755","/bin/","qmail-inject");
  copy(auto_uido,"755","/bin/","predate");
  copy(auto_uido,"755","/bin/","datemail");
  copy(auto_uido,"755","/bin/","mailsubj");
  copy(auto_uido,"755","/bin/","qmail-showctl");
  copy(auto_uido,"755","/bin/","qmail-qread");
  copy(auto_uido,"755","/bin/","qmail-qstat");
  copy(auto_uido,"755","/bin/","qmail-tcpto");
  copy(auto_uido,"755","/bin/","qmail-tcpok");
  copy(auto_uido,"755","/bin/","qmail-pop3d");
  copy(auto_uido,"711","/bin/","qmail-popup");
  copy(auto_uido,"755","/bin/","qmail-qmqpc");
  copy(auto_uido,"755","/bin/","qmail-qmqpd");
  copy(auto_uido,"755","/bin/","qmail-qmtpd");
  copy(auto_uido,"755","/bin/","qmail-smtpd");
  copy(auto_uido,"755","/bin/","sendmail");
  copy(auto_uido,"755","/bin/","tcp-env");
  copy(auto_uido,"755","/bin/","qreceipt");
  copy(auto_uido,"755","/bin/","qsmhook");
  copy(auto_uido,"755","/bin/","qbiff");
  copy(auto_uido,"755","/bin/","forward");
  copy(auto_uido,"755","/bin/","preline");
  copy(auto_uido,"755","/bin/","condredirect");
  copy(auto_uido,"755","/bin/","maildirmake");
  copy(auto_uido,"755","/bin/","maildir2mbox");
  copy(auto_uido,"755","/bin/","maildirwatch");
  copy(auto_uido,"755","/bin/","qail");
  copy(auto_uido,"755","/bin/","elq");
  copy(auto_uido,"755","/bin/","pinq");

  copy(auto_uido,"644","/man/man5/","addresses.5");
  copy(auto_uido,"644","/man/cat5/","addresses.0");
  copy(auto_uido,"644","/man/man5/","envelopes.5");
  copy(auto_uido,"644","/man/cat5/","envelopes.0");
  copy(auto_uido,"644","/man/man5/","maildir.5");
  copy(auto_uido,"644","/man/cat5/","maildir.0");
  copy(auto_uido,"644","/man/man5/","mbox.5");
  copy(auto_uido,"644","/man/cat5/","mbox.0");
  copy(auto_uido,"644","/man/man5/","dot-qmail.5");
  copy(auto_uido,"644","/man/cat5/","dot-qmail.0");
  copy(auto_uido,"644","/man/man5/","qmail-control.5");
  copy(auto_uido,"644","/man/cat5/","qmail-control.0");
  copy(auto_uido,"644","/man/man5/","qmail-header.5");
  copy(auto_uido,"644","/man/cat5/","qmail-header.0");
  copy(auto_uido,"644","/man/man5/","qmail-log.5");
  copy(auto_uido,"644","/man/cat5/","qmail-log.0");
  copy(auto_uido,"644","/man/man5/","qmail-users.5");
  copy(auto_uido,"644","/man/cat5/","qmail-users.0");
  copy(auto_uido,"644","/man/man5/","tcp-environ.5");
  copy(auto_uido,"644","/man/cat5/","tcp-environ.0");

  copy(auto_uido,"644","/man/man7/","forgeries.7");
  copy(auto_uido,"644","/man/cat7/","forgeries.0");
  copy(auto_uido,"644","/man/man7/","qmail-limits.7");
  copy(auto_uido,"644","/man/cat7/","qmail-limits.0");
  copy(auto_uido,"644","/man/man7/","qmail.7");
  copy(auto_uido,"644","/man/cat7/","qmail.0");

  copy(auto_uido,"644","/man/man1/","forward.1");
  copy(auto_uido,"644","/man/cat1/","forward.0");
  copy(auto_uido,"644","/man/man1/","condredirect.1");
  copy(auto_uido,"644","/man/cat1/","condredirect.0");
  copy(auto_uido,"644","/man/man1/","maildirmake.1");
  copy(auto_uido,"644","/man/cat1/","maildirmake.0");
  copy(auto_uido,"644","/man/man1/","maildir2mbox.1");
  copy(auto_uido,"644","/man/cat1/","maildir2mbox.0");
  copy(auto_uido,"644","/man/man1/","maildirwatch.1");
  copy(auto_uido,"644","/man/cat1/","maildirwatch.0");
  copy(auto_uido,"644","/man/man1/","mailsubj.1");
  copy(auto_uido,"644","/man/cat1/","mailsubj.0");
  copy(auto_uido,"644","/man/man1/","qreceipt.1");
  copy(auto_uido,"644","/man/cat1/","qreceipt.0");
  copy(auto_uido,"644","/man/man1/","qbiff.1");
  copy(auto_uido,"644","/man/cat1/","qbiff.0");
  copy(auto_uido,"644","/man/man1/","preline.1");
  copy(auto_uido,"644","/man/cat1/","preline.0");
  copy(auto_uido,"644","/man/man1/","tcp-env.1");
  copy(auto_uido,"644","/man/cat1/","tcp-env.0");

  copy(auto_uido,"644","/man/man8/","qmail-local.8");
  copy(auto_uido,"644","/man/cat8/","qmail-local.0");
  copy(auto_uido,"644","/man/man8/","qmail-lspawn.8");
  copy(auto_uido,"644","/man/cat8/","qmail-lspawn.0");
  copy(auto_uido,"644","/man/man8/","qmail-getpw.8");
  copy(auto_uido,"644","/man/cat8/","qmail-getpw.0");
  copy(auto_uido,"644","/man/man8/","qmail-remote.8");
  copy(auto_uido,"644","/man/cat8/","qmail-remote.0");
  copy(auto_uido,"644","/man/man8/","qmail-rspawn.8");
  copy(auto_uido,"644","/man/cat8/","qmail-rspawn.0");
  copy(auto_uido,"644","/man/man8/","qmail-clean.8");
  copy(auto_uido,"644","/man/cat8/","qmail-clean.0");
  copy(auto_uido,"644","/man/man8/","qmail-send.8");
  copy(auto_uido,"644","/man/cat8/","qmail-send.0");
  copy(auto_uido,"644","/man/man8/","qmail-start.8");
  copy(auto_uido,"644","/man/cat8/","qmail-start.0");
  copy(auto_uido,"644","/man/man8/","splogger.8");
  copy(auto_uido,"644","/man/cat8/","splogger.0");
  copy(auto_uido,"644","/man/man8/","qmail-queue.8");
  copy(auto_uido,"644","/man/cat8/","qmail-queue.0");
  copy(auto_uido,"644","/man/man8/","qmail-inject.8");
  copy(auto_uido,"644","/man/cat8/","qmail-inject.0");
  copy(auto_uido,"644","/man/man8/","qmail-showctl.8");
  copy(auto_uido,"644","/man/cat8/","qmail-showctl.0");
  copy(auto_uido,"644","/man/man8/","qmail-newmrh.8");
  copy(auto_uido,"644","/man/cat8/","qmail-newmrh.0");
  copy(auto_uido,"644","/man/man8/","qmail-newu.8");
  copy(auto_uido,"644","/man/cat8/","qmail-newu.0");
  copy(auto_uido,"644","/man/man8/","qmail-pw2u.8");
  copy(auto_uido,"644","/man/cat8/","qmail-pw2u.0");
  copy(auto_uido,"644","/man/man8/","qmail-qread.8");
  copy(auto_uido,"644","/man/cat8/","qmail-qread.0");
  copy(auto_uido,"644","/man/man8/","qmail-qstat.8");
  copy(auto_uido,"644","/man/cat8/","qmail-qstat.0");
  copy(auto_uido,"644","/man/man8/","qmail-tcpok.8");
  copy(auto_uido,"644","/man/cat8/","qmail-tcpok.0");
  copy(auto_uido,"644","/man/man8/","qmail-tcpto.8");
  copy(auto_uido,"644","/man/cat8/","qmail-tcpto.0");
  copy(auto_uido,"644","/man/man8/","qmail-pop3d.8");
  copy(auto_uido,"644","/man/cat8/","qmail-pop3d.0");
  copy(auto_uido,"644","/man/man8/","qmail-popup.8");
  copy(auto_uido,"644","/man/cat8/","qmail-popup.0");
  copy(auto_uido,"644","/man/man8/","qmail-qmqpc.8");
  copy(auto_uido,"644","/man/cat8/","qmail-qmqpc.0");
  copy(auto_uido,"644","/man/man8/","qmail-qmqpd.8");
  copy(auto_uido,"644","/man/cat8/","qmail-qmqpd.0");
  copy(auto_uido,"644","/man/man8/","qmail-qmtpd.8");
  copy(auto_uido,"644","/man/cat8/","qmail-qmtpd.0");
  copy(auto_uido,"644","/man/man8/","qmail-smtpd.8");
  copy(auto_uido,"644","/man/cat8/","qmail-smtpd.0");
  copy(auto_uido,"644","/man/man8/","qmail-command.8");
  copy(auto_uido,"644","/man/cat8/","qmail-command.0");

  substdio_flush(subfdout);
  _exit(0);
}
