#include "sig.h"
#include "env.h"
#include "substdio.h"
#include "stralloc.h"
#include "subfd.h"
#include "getln.h"
#include "alloc.h"
#include "str.h"
#include "hfield.h"
#include "token822.h"
#include "error.h"
#include "gen_alloc.h"
#include "gen_allocdefs.h"
#include "headerbody.h"
#include "exit.h"
#include "open.h"
#include "quote.h"
#include "qmail.h"

void die_noreceipt() { _exit(0); }
void die() { _exit(100); }
void die_temp() { _exit(111); }
void die_nomem() {
 substdio_putsflush(subfderr,"qreceipt: fatal: out of memory\n"); die_temp(); }
void die_fork() {
 substdio_putsflush(subfderr,"qreceipt: fatal: unable to fork\n"); die_temp(); }
void die_qqperm() {
 substdio_putsflush(subfderr,"qreceipt: fatal: permanent qmail-queue error\n"); die(); }
void die_qqtemp() {
 substdio_putsflush(subfderr,"qreceipt: fatal: temporary qmail-queue error\n"); die_temp(); }
void die_usage() {
 substdio_putsflush(subfderr,
 "qreceipt: usage: qreceipt deliveryaddress\n"); die(); }
void die_read() {
 if (errno == error_nomem) die_nomem();
 substdio_putsflush(subfderr,"qreceipt: fatal: read error\n"); die_temp(); }
void doordie(sa,r) stralloc *sa; int r; {
 if (r == 1) return; if (r == -1) die_nomem();
 substdio_putsflush(subfderr,"qreceipt: fatal: unable to parse this: ");
 substdio_putflush(subfderr,sa->s,sa->len); die(); }

char *target;

int flagreceipt = 0;

char *returnpath;
stralloc messageid = {0};
stralloc sanotice = {0};

int rwnotice(addr) token822_alloc *addr; { token822_reverse(addr);
 if (token822_unquote(&sanotice,addr) != 1) die_nomem();
 if (sanotice.len == str_len(target))
   if (!str_diffn(sanotice.s,target,sanotice.len))
     flagreceipt = 1;
 token822_reverse(addr); return 1; }

struct qmail qqt;

stralloc quoted = {0};

void finishheader()
{
 char *qqx;

 if (!flagreceipt) die_noreceipt();
 if (str_equal(returnpath,"")) die_noreceipt();
 if (str_equal(returnpath,"#@[]")) die_noreceipt();

 if (!quote2(&quoted,returnpath)) die_nomem();

 if (qmail_open(&qqt) == -1) die_fork();

 qmail_puts(&qqt,"From: DELIVERY NOTICE SYSTEM <");
 qmail_put(&qqt,quoted.s,quoted.len);
 qmail_puts(&qqt,">\n");
 qmail_puts(&qqt,"To: <");
 qmail_put(&qqt,quoted.s,quoted.len);
 qmail_puts(&qqt,">\n");
 qmail_puts(&qqt,"Subject: success notice\n\
\n\
Hi! This is the qreceipt program. Your message was delivered to the\n\
following address: ");
 qmail_puts(&qqt,target);
 qmail_puts(&qqt,". Thanks for asking.\n");
 if (messageid.s)
  {
   qmail_puts(&qqt,"Your ");
   qmail_put(&qqt,messageid.s,messageid.len);
  }

 qmail_from(&qqt,"");
 qmail_to(&qqt,returnpath);
 qqx = qmail_close(&qqt);

 if (*qqx)
   if (*qqx == 'D') die_qqperm();
   else die_qqtemp();
}

stralloc hfbuf = {0};
token822_alloc hfin = {0};
token822_alloc hfrewrite = {0};
token822_alloc hfaddr = {0};

void doheaderfield(h)
stralloc *h;
{
 switch(hfield_known(h->s,h->len))
  {
   case H_MESSAGEID:
     if (!stralloc_copy(&messageid,h)) die_nomem();
     break;
   case H_NOTICEREQUESTEDUPONDELIVERYTO:
     doordie(h,token822_parse(&hfin,h,&hfbuf));
     doordie(h,token822_addrlist(&hfrewrite,&hfaddr,&hfin,rwnotice));
     break;
  }
}

void dobody(h) stralloc *h; { ; }

void main(argc,argv)
int argc;
char **argv;
{
 sig_pipeignore();
 if (!(target = argv[1])) die_usage();
 if (!(returnpath = env_get("SENDER"))) die_usage();
 if (headerbody(subfdin,doheaderfield,finishheader,dobody) == -1) die_read();
 die_noreceipt();
}
