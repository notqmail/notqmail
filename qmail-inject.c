#include "sig.h"
#include "substdio.h"
#include "stralloc.h"
#include "subfd.h"
#include "sgetopt.h"
#include "getln.h"
#include "alloc.h"
#include "str.h"
#include "fmt.h"
#include "hfield.h"
#include "token822.h"
#include "control.h"
#include "env.h"
#include "gen_alloc.h"
#include "gen_allocdefs.h"
#include "error.h"
#include "qmail.h"
#include "now.h"
#include "exit.h"
#include "quote.h"
#include "headerbody.h"
#include "auto_qmail.h"
#include "newfield.h"
#include "constmap.h"

#define LINELEN 80

datetime_sec starttime;

char *qmopts;
int flagdeletesender = 0;
int flagdeletefrom = 0;
int flagdeletemessid = 0;
int flagnamecomment = 0;
int flaghackmess = 0;
int flaghackrecip = 0;
char *mailhost;
char *mailuser;
int mailusertokentype;
char *mailrhost;
char *mailruser;

stralloc control_idhost = {0};
stralloc control_defaultdomain = {0};
stralloc control_defaulthost = {0};
stralloc control_plusdomain = {0};

stralloc sender = {0};
stralloc envsbuf = {0};
token822_alloc envs = {0};
int flagrh;

int flagqueue;
struct qmail qqt;

void put(s,len) char *s; int len;
{ if (flagqueue) qmail_put(&qqt,s,len); else substdio_put(subfdout,s,len); }
void puts(s) char *s; { put(s,str_len(s)); }

void perm() { _exit(100); }
void temp() { _exit(111); }
void die_nomem() {
 substdio_putsflush(subfderr,"qmail-inject: fatal: out of memory\n"); temp(); }
void die_invalid(sa) stralloc *sa; {
 substdio_putsflush(subfderr,"qmail-inject: fatal: invalid header field: ");
 substdio_putflush(subfderr,sa->s,sa->len); perm(); }
void die_qqt() {
 substdio_putsflush(subfderr,"qmail-inject: fatal: unable to run qmail-queue\n"); temp(); }
void die_chdir() {
 substdio_putsflush(subfderr,"qmail-inject: fatal: internal bug\n"); temp(); }
void die_read() {
 if (errno == error_nomem) die_nomem();
 substdio_putsflush(subfderr,"qmail-inject: fatal: read error\n"); temp(); }
void doordie(sa,r) stralloc *sa; int r; {
 if (r == 1) return; if (r == -1) die_nomem();
 substdio_putsflush(subfderr,"qmail-inject: fatal: unable to parse this line:\n");
 substdio_putflush(subfderr,sa->s,sa->len); perm(); }

GEN_ALLOC_typedef(saa,stralloc,sa,len,a)
GEN_ALLOC_readyplus(saa,stralloc,sa,len,a,i,n,x,10,saa_readyplus)

static stralloc sauninit = {0};

saa savedh = {0};
saa hrlist = {0};
saa tocclist = {0};
saa hrrlist = {0};
saa reciplist = {0};
int flagresent;

void exitnicely()
{
 char *qqx;

 if (!flagqueue) substdio_flush(subfdout);

 if (flagqueue)
  {
   int i;

   if (!stralloc_0(&sender)) die_nomem();
   qmail_from(&qqt,sender.s);

   for (i = 0;i < reciplist.len;++i)
    {
     if (!stralloc_0(&reciplist.sa[i])) die_nomem();
     qmail_to(&qqt,reciplist.sa[i].s);
    }
   if (flagrh)
     if (flagresent)
       for (i = 0;i < hrrlist.len;++i)
	{
         if (!stralloc_0(&hrrlist.sa[i])) die_nomem();
	 qmail_to(&qqt,hrrlist.sa[i].s);
	}
     else
       for (i = 0;i < hrlist.len;++i)
	{
         if (!stralloc_0(&hrlist.sa[i])) die_nomem();
	 qmail_to(&qqt,hrlist.sa[i].s);
	}

   qqx = qmail_close(&qqt);
   if (*qqx)
     if (*qqx == 'D') {
       substdio_puts(subfderr,"qmail-inject: fatal: ");
       substdio_puts(subfderr,qqx + 1);
       substdio_puts(subfderr,"\n");
       substdio_flush(subfderr);
       perm();
     }
     else {
       substdio_puts(subfderr,"qmail-inject: fatal: ");
       substdio_puts(subfderr,qqx + 1);
       substdio_puts(subfderr,"\n");
       substdio_flush(subfderr);
       temp();
     }
  }

 _exit(0);
}

void savedh_append(h)
stralloc *h;
{
 if (!saa_readyplus(&savedh,1)) die_nomem();
 savedh.sa[savedh.len] = sauninit;
 if (!stralloc_copy(savedh.sa + savedh.len,h)) die_nomem();
 ++savedh.len;
}

void savedh_print()
{
 int i;

 for (i = 0;i < savedh.len;++i)
   put(savedh.sa[i].s,savedh.sa[i].len);
}

stralloc defaultdomainbuf = {0};
token822_alloc defaultdomain = {0};
stralloc defaulthostbuf = {0};
token822_alloc defaulthost = {0};
stralloc plusdomainbuf = {0};
token822_alloc plusdomain = {0};

void rwroute(addr)
token822_alloc *addr;
{
 if (addr->t[addr->len - 1].type == TOKEN822_AT)
   while (addr->len)
     if (addr->t[--addr->len].type == TOKEN822_COLON)
       return;
}

void rwextraat(addr)
token822_alloc *addr;
{
 int i;
 if (addr->t[0].type == TOKEN822_AT)
  {
   --addr->len;
   for (i = 0;i < addr->len;++i)
     addr->t[i] = addr->t[i + 1];
  }
}

void rwextradot(addr)
token822_alloc *addr;
{
 int i;
 if (addr->t[0].type == TOKEN822_DOT)
  {
   --addr->len;
   for (i = 0;i < addr->len;++i)
     addr->t[i] = addr->t[i + 1];
  }
}

void rwnoat(addr)
token822_alloc *addr;
{
 int i;
 int shift;

 for (i = 0;i < addr->len;++i)
   if (addr->t[i].type == TOKEN822_AT)
     return;
 shift = defaulthost.len;
 if (!token822_readyplus(addr,shift)) die_nomem();
 for (i = addr->len - 1;i >= 0;--i)
   addr->t[i + shift] = addr->t[i];
 addr->len += shift;
 for (i = 0;i < shift;++i)
   addr->t[i] = defaulthost.t[shift - 1 - i];
}

void rwnodot(addr)
token822_alloc *addr;
{
 int i;
 int shift;
 for (i = 0;i < addr->len;++i)
  {
   if (addr->t[i].type == TOKEN822_DOT)
     return;
   if (addr->t[i].type == TOKEN822_AT)
     break;
  }
 for (i = 0;i < addr->len;++i)
  {
   if (addr->t[i].type == TOKEN822_LITERAL)
     return;
   if (addr->t[i].type == TOKEN822_AT)
     break;
  }
 shift = defaultdomain.len;
 if (!token822_readyplus(addr,shift)) die_nomem();
 for (i = addr->len - 1;i >= 0;--i)
   addr->t[i + shift] = addr->t[i];
 addr->len += shift;
 for (i = 0;i < shift;++i)
   addr->t[i] = defaultdomain.t[shift - 1 - i];
}

void rwplus(addr)
token822_alloc *addr;
{
 int i;
 int shift;

 if (addr->t[0].type != TOKEN822_ATOM) return;
 if (!addr->t[0].slen) return;
 if (addr->t[0].s[addr->t[0].slen - 1] != '+') return;

 --addr->t[0].slen; /* remove + */

 shift = plusdomain.len;
 if (!token822_readyplus(addr,shift)) die_nomem();
 for (i = addr->len - 1;i >= 0;--i)
   addr->t[i + shift] = addr->t[i];
 addr->len += shift;
 for (i = 0;i < shift;++i)
   addr->t[i] = plusdomain.t[shift - 1 - i];
}

void rwgeneric(addr)
token822_alloc *addr;
{
 if (!addr->len) return; /* don't rewrite <> */
 if (addr->len >= 2)
   if (addr->t[1].type == TOKEN822_AT)
     if (addr->t[0].type == TOKEN822_LITERAL)
       if (!addr->t[0].slen) /* don't rewrite <foo@[]> */
	 return;
 rwroute(addr);
 if (!addr->len) return; /* <@foo:> -> <> */
 rwextradot(addr);
 if (!addr->len) return; /* <.> -> <> */
 rwextraat(addr);
 if (!addr->len) return; /* <@> -> <> */
 rwnoat(addr);
 rwplus(addr);
 rwnodot(addr);
}

int setreturn(addr)
token822_alloc *addr;
{
 if (!sender.s)
  {
   token822_reverse(addr);
   if (token822_unquote(&sender,addr) != 1) die_nomem();
   if (flaghackrecip)
     if (!stralloc_cats(&sender,"-@[]")) die_nomem();
   token822_reverse(addr);
  }
 return 1;
}

int rwreturn(addr)
token822_alloc *addr;
{
 rwgeneric(addr);
 setreturn(addr);
 return 1;
}

int rwsender(addr)
token822_alloc *addr;
{
 rwgeneric(addr);
 return 1;
}

void rwappend(addr,xl)
token822_alloc *addr;
saa *xl;
{
 token822_reverse(addr);
 if (!saa_readyplus(xl,1)) die_nomem();
 xl->sa[xl->len] = sauninit;
 if (token822_unquote(&xl->sa[xl->len],addr) != 1) die_nomem();
 ++xl->len;
 token822_reverse(addr);
}

int rwhrr(addr) token822_alloc *addr;
{ rwgeneric(addr); rwappend(addr,&hrrlist); return 1; }
int rwhr(addr) token822_alloc *addr;
{ rwgeneric(addr); rwappend(addr,&hrlist); return 1; }
int rwtocc(addr) token822_alloc *addr;
{ rwgeneric(addr); rwappend(addr,&hrlist); rwappend(addr,&tocclist); return 1; }

int htypeseen[H_NUM];
stralloc hfbuf = {0};
token822_alloc hfin = {0};
token822_alloc hfrewrite = {0};
token822_alloc hfaddr = {0};

void doheaderfield(h)
stralloc *h;
{
  int htype;
  int (*rw)() = 0;
 
  htype = hfield_known(h->s,h->len);
  if (flagdeletefrom) if (htype == H_FROM) return;
  if (flagdeletemessid) if (htype == H_MESSAGEID) return;
  if (flagdeletesender) if (htype == H_RETURNPATH) return;
 
  if (htype)
    htypeseen[htype] = 1;
  else
    if (!hfield_valid(h->s,h->len))
      die_invalid(h);
 
  switch(htype) {
    case H_TO: case H_CC:
      rw = rwtocc; break;
    case H_BCC: case H_APPARENTLYTO:
      rw = rwhr; break;
    case H_R_TO: case H_R_CC: case H_R_BCC:
      rw = rwhrr; break;
    case H_RETURNPATH:
      rw = rwreturn; break;
    case H_SENDER: case H_FROM: case H_REPLYTO:
    case H_RETURNRECEIPTTO: case H_ERRORSTO:
    case H_R_SENDER: case H_R_FROM: case H_R_REPLYTO:
      rw = rwsender; break;
  }

  if (rw) {
    doordie(h,token822_parse(&hfin,h,&hfbuf));
    doordie(h,token822_addrlist(&hfrewrite,&hfaddr,&hfin,rw));
    if (token822_unparse(h,&hfrewrite,LINELEN) != 1)
      die_nomem();
  }
 
  if (htype == H_BCC) return;
  if (htype == H_R_BCC) return;
  if (htype == H_RETURNPATH) return;
  if (htype == H_CONTENTLENGTH) return; /* some things are just too stupid */
  savedh_append(h);
}

void dobody(h)
stralloc *h;
{
 put(h->s,h->len);
}

stralloc torecip = {0};
token822_alloc tr = {0};

void dorecip(s)
char *s;
{
 if (!quote2(&torecip,s)) die_nomem();
 switch(token822_parse(&tr,&torecip,&hfbuf))
  {
   case -1: die_nomem();
   case 0:
     substdio_puts(subfderr,"qmail-inject: fatal: unable to parse address: ");
     substdio_puts(subfderr,s);
     substdio_putsflush(subfderr,"\n");
     perm();
  }
 token822_reverse(&tr);
 rwgeneric(&tr);
 rwappend(&tr,&reciplist);
}

stralloc defaultfrom = {0};
token822_alloc df = {0};

void defaultfrommake()
{
 char *fullname;
 fullname = env_get("QMAILNAME");
 if (!fullname) fullname = env_get("MAILNAME");
 if (!fullname) fullname = env_get("NAME");
 if (!token822_ready(&df,20)) die_nomem();
 df.len = 0;
 df.t[df.len].type = TOKEN822_ATOM;
 df.t[df.len].s = "From";
 df.t[df.len].slen = 4;
 ++df.len;
 df.t[df.len].type = TOKEN822_COLON;
 ++df.len;
 if (fullname && !flagnamecomment)
  {
   df.t[df.len].type = TOKEN822_QUOTE;
   df.t[df.len].s = fullname;
   df.t[df.len].slen = str_len(fullname);
   ++df.len;
   df.t[df.len].type = TOKEN822_LEFT;
   ++df.len;
  }
 df.t[df.len].type = mailusertokentype;
 df.t[df.len].s = mailuser;
 df.t[df.len].slen = str_len(mailuser);
 ++df.len;
 if (mailhost)
  {
   df.t[df.len].type = TOKEN822_AT;
   ++df.len;
   df.t[df.len].type = TOKEN822_ATOM;
   df.t[df.len].s = mailhost;
   df.t[df.len].slen = str_len(mailhost);
   ++df.len;
  }
 if (fullname && !flagnamecomment)
  {
   df.t[df.len].type = TOKEN822_RIGHT;
   ++df.len;
  }
 if (fullname && flagnamecomment)
  {
   df.t[df.len].type = TOKEN822_COMMENT;
   df.t[df.len].s = fullname;
   df.t[df.len].slen = str_len(fullname);
   ++df.len;
  }
 if (token822_unparse(&defaultfrom,&df,LINELEN) != 1) die_nomem();
 doordie(&defaultfrom,token822_parse(&df,&defaultfrom,&hfbuf));
 doordie(&defaultfrom,token822_addrlist(&hfrewrite,&hfaddr,&df,rwsender));
 if (token822_unparse(&defaultfrom,&hfrewrite,LINELEN) != 1) die_nomem();
}

stralloc defaultreturnpath = {0};
token822_alloc drp = {0};
stralloc hackedruser = {0};
char strnum[FMT_ULONG];

void dodefaultreturnpath()
{
 if (!stralloc_copys(&hackedruser,mailruser)) die_nomem();
 if (flaghackmess)
  {
   if (!stralloc_cats(&hackedruser,"-")) die_nomem();
   if (!stralloc_catb(&hackedruser,strnum,fmt_ulong(strnum,(unsigned long) starttime))) die_nomem();
   if (!stralloc_cats(&hackedruser,".")) die_nomem();
   if (!stralloc_catb(&hackedruser,strnum,fmt_ulong(strnum,(unsigned long) getpid()))) die_nomem();
  }
 if (flaghackrecip)
   if (!stralloc_cats(&hackedruser,"-")) die_nomem();
 if (!token822_ready(&drp,10)) die_nomem();
 drp.len = 0;
 drp.t[drp.len].type = TOKEN822_ATOM;
 drp.t[drp.len].s = "Return-Path";
 drp.t[drp.len].slen = 11;
 ++drp.len;
 drp.t[drp.len].type = TOKEN822_COLON;
 ++drp.len;
 drp.t[drp.len].type = TOKEN822_QUOTE;
 drp.t[drp.len].s = hackedruser.s;
 drp.t[drp.len].slen = hackedruser.len;
 ++drp.len;
 if (mailrhost)
  {
   drp.t[drp.len].type = TOKEN822_AT;
   ++drp.len;
   drp.t[drp.len].type = TOKEN822_ATOM;
   drp.t[drp.len].s = mailrhost;
   drp.t[drp.len].slen = str_len(mailrhost);
   ++drp.len;
  }
 if (token822_unparse(&defaultreturnpath,&drp,LINELEN) != 1) die_nomem();
 doordie(&defaultreturnpath,token822_parse(&drp,&defaultreturnpath,&hfbuf));
 doordie(&defaultreturnpath
   ,token822_addrlist(&hfrewrite,&hfaddr,&drp,rwreturn));
 if (token822_unparse(&defaultreturnpath,&hfrewrite,LINELEN) != 1) die_nomem();
}

int flagmft = 0;
stralloc mft = {0};
struct constmap mapmft;

void mft_init()
{
  char *x;
  int r;

  x = env_get("QMAILMFTFILE");
  if (!x) return;

  r = control_readfile(&mft,x,0);
  if (r == -1) die_read(); /*XXX*/
  if (!r) return;

  if (!constmap_init(&mapmft,mft.s,mft.len,0)) die_nomem();
  flagmft = 1;
}

void finishmft()
{
  int i;
  static stralloc sa = {0};
  static stralloc sa2 = {0};

  if (!flagmft) return;
  if (htypeseen[H_MAILFOLLOWUPTO]) return;

  for (i = 0;i < tocclist.len;++i)
    if (constmap(&mapmft,tocclist.sa[i].s,tocclist.sa[i].len))
      break;

  if (i == tocclist.len) return;

  puts("Mail-Followup-To: ");
  i = tocclist.len;
  while (i--) {
    if (!stralloc_copy(&sa,&tocclist.sa[i])) die_nomem();
    if (!stralloc_0(&sa)) die_nomem();
    if (!quote2(&sa2,sa.s)) die_nomem();
    put(sa2.s,sa2.len);
    if (i) puts(",\n  ");
  }
  puts("\n");
}

void finishheader()
{
 flagresent =
   htypeseen[H_R_SENDER] || htypeseen[H_R_FROM] || htypeseen[H_R_REPLYTO]
   || htypeseen[H_R_TO] || htypeseen[H_R_CC] || htypeseen[H_R_BCC]
   || htypeseen[H_R_DATE] || htypeseen[H_R_MESSAGEID];

 if (!sender.s)
   dodefaultreturnpath();

 if (!flagqueue)
  {
   static stralloc sa = {0};
   static stralloc sa2 = {0};

   if (!stralloc_copy(&sa,&sender)) die_nomem();
   if (!stralloc_0(&sa)) die_nomem();
   if (!quote2(&sa2,sa.s)) die_nomem();

   puts("Return-Path: <");
   put(sa2.s,sa2.len);
   puts(">\n");
  }

 /* could check at this point whether there are any recipients */
 if (flagqueue)
   if (qmail_open(&qqt) == -1) die_qqt();

 if (flagresent)
  {
   if (!htypeseen[H_R_DATE])
    {
     if (!newfield_datemake(starttime)) die_nomem();
     puts("Resent-");
     put(newfield_date.s,newfield_date.len);
    }
   if (!htypeseen[H_R_MESSAGEID])
    {
     if (!newfield_msgidmake(control_idhost.s,control_idhost.len,starttime)) die_nomem();
     puts("Resent-");
     put(newfield_msgid.s,newfield_msgid.len);
    }
   if (!htypeseen[H_R_FROM])
    {
     defaultfrommake();
     puts("Resent-");
     put(defaultfrom.s,defaultfrom.len);
    }
   if (!htypeseen[H_R_TO] && !htypeseen[H_R_CC])
     puts("Resent-Cc: recipient list not shown: ;\n");
  }
 else
  {
   if (!htypeseen[H_DATE])
    {
     if (!newfield_datemake(starttime)) die_nomem();
     put(newfield_date.s,newfield_date.len);
    }
   if (!htypeseen[H_MESSAGEID])
    {
     if (!newfield_msgidmake(control_idhost.s,control_idhost.len,starttime)) die_nomem();
     put(newfield_msgid.s,newfield_msgid.len);
    }
   if (!htypeseen[H_FROM])
    {
     defaultfrommake();
     put(defaultfrom.s,defaultfrom.len);
    }
   if (!htypeseen[H_TO] && !htypeseen[H_CC])
     puts("Cc: recipient list not shown: ;\n");
   finishmft();
  }

 savedh_print();
}

void getcontrols()
{
 static stralloc sa = {0};
 char *x;

 mft_init();

 if (chdir(auto_qmail) == -1) die_chdir();
 if (control_init() == -1) die_read();

 if (control_rldef(&control_defaultdomain,"control/defaultdomain",1,"defaultdomain") != 1)
   die_read();
 x = env_get("QMAILDEFAULTDOMAIN");
 if (x) if (!stralloc_copys(&control_defaultdomain,x)) die_nomem();
 if (!stralloc_copys(&sa,".")) die_nomem();
 if (!stralloc_cat(&sa,&control_defaultdomain)) die_nomem();
 doordie(&sa,token822_parse(&defaultdomain,&sa,&defaultdomainbuf));

 if (control_rldef(&control_defaulthost,"control/defaulthost",1,"defaulthost") != 1)
   die_read();
 x = env_get("QMAILDEFAULTHOST");
 if (x) if (!stralloc_copys(&control_defaulthost,x)) die_nomem();
 if (!stralloc_copys(&sa,"@")) die_nomem();
 if (!stralloc_cat(&sa,&control_defaulthost)) die_nomem();
 doordie(&sa,token822_parse(&defaulthost,&sa,&defaulthostbuf));

 if (control_rldef(&control_plusdomain,"control/plusdomain",1,"plusdomain") != 1)
   die_read();
 x = env_get("QMAILPLUSDOMAIN");
 if (x) if (!stralloc_copys(&control_plusdomain,x)) die_nomem();
 if (!stralloc_copys(&sa,".")) die_nomem();
 if (!stralloc_cat(&sa,&control_plusdomain)) die_nomem();
 doordie(&sa,token822_parse(&plusdomain,&sa,&plusdomainbuf));

 if (control_rldef(&control_idhost,"control/idhost",1,"idhost") != 1)
   die_read();
 x = env_get("QMAILIDHOST");
 if (x) if (!stralloc_copys(&control_idhost,x)) die_nomem();
}

#define RECIP_DEFAULT 1
#define RECIP_ARGS 2
#define RECIP_HEADER 3
#define RECIP_AH 4

void main(argc,argv)
int argc;
char **argv;
{
 int i;
 int opt;
 int recipstrategy;

 sig_pipeignore();

 starttime = now();

 qmopts = env_get("QMAILINJECT");
 if (qmopts)
   while (*qmopts)
     switch(*qmopts++)
      {
       case 'c': flagnamecomment = 1; break;
       case 's': flagdeletesender = 1; break;
       case 'f': flagdeletefrom = 1; break;
       case 'i': flagdeletemessid = 1; break;
       case 'r': flaghackrecip = 1; break;
       case 'm': flaghackmess = 1; break;
      }

 mailhost = env_get("QMAILHOST");
 if (!mailhost) mailhost = env_get("MAILHOST");
 mailrhost = env_get("QMAILSHOST");
 if (!mailrhost) mailrhost = mailhost;

 mailuser = env_get("QMAILUSER");
 if (!mailuser) mailuser = env_get("MAILUSER");
 if (!mailuser) mailuser = env_get("USER");
 if (!mailuser) mailuser = env_get("LOGNAME");
 if (!mailuser) mailuser = "anonymous";
 mailusertokentype = TOKEN822_ATOM;
 if (quote_need(mailuser,str_len(mailuser))) mailusertokentype = TOKEN822_QUOTE;
 mailruser = env_get("QMAILSUSER");
 if (!mailruser) mailruser = mailuser;

 for (i = 0;i < H_NUM;++i) htypeseen[i] = 0;

 recipstrategy = RECIP_DEFAULT;
 flagqueue = 1;

 getcontrols();

 if (!saa_readyplus(&hrlist,1)) die_nomem();
 if (!saa_readyplus(&tocclist,1)) die_nomem();
 if (!saa_readyplus(&hrrlist,1)) die_nomem();
 if (!saa_readyplus(&reciplist,1)) die_nomem();

 while ((opt = getopt(argc,argv,"aAhHnNf:")) != opteof)
   switch(opt)
    {
     case 'a': recipstrategy = RECIP_ARGS; break;
     case 'A': recipstrategy = RECIP_DEFAULT; break;
     case 'h': recipstrategy = RECIP_HEADER; break;
     case 'H': recipstrategy = RECIP_AH; break;
     case 'n': flagqueue = 0; break;
     case 'N': flagqueue = 1; break;
     case 'f':
       if (!quote2(&sender,optarg)) die_nomem();
       doordie(&sender,token822_parse(&envs,&sender,&envsbuf));
       token822_reverse(&envs);
       rwgeneric(&envs);
       token822_reverse(&envs);
       if (token822_unquote(&sender,&envs) != 1) die_nomem();
       break;
     case '?':
     default:
       perm();
    }
 argc -= optind;
 argv += optind;

 if (recipstrategy == RECIP_DEFAULT)
   recipstrategy = (*argv ? RECIP_ARGS : RECIP_HEADER);

 if (recipstrategy != RECIP_HEADER)
   while (*argv)
     dorecip(*argv++);

 flagrh = (recipstrategy != RECIP_ARGS);

 if (headerbody(subfdin,doheaderfield,finishheader,dobody) == -1)
   die_read();
 exitnicely();
}
