#include <sys/types.h>
#include <sys/stat.h>
#include "substdio.h"
#include "readwrite.h"
#include "subfd.h"
#include "sgetopt.h"
#include "control.h"
#include "constmap.h"
#include "stralloc.h"
#include "fmt.h"
#include "str.h"
#include "scan.h"
#include "open.h"
#include "error.h"
#include "getln.h"
#include "auto_break.h"
#include "auto_qmail.h"
#include "auto_usera.h"

void die_chdir()
{
  substdio_putsflush(subfderr,"qmail-pw2u: fatal: unable to chdir\n");
  _exit(111);
}
void die_nomem()
{
  substdio_putsflush(subfderr,"qmail-pw2u: fatal: out of memory\n");
  _exit(111);
}
void die_read()
{
  substdio_putsflush(subfderr,"qmail-pw2u: fatal: unable to read input\n");
  _exit(111);
}
void die_write()
{
  substdio_putsflush(subfderr,"qmail-pw2u: fatal: unable to write output\n");
  _exit(111);
}
void die_control()
{
  substdio_putsflush(subfderr,"qmail-pw2u: fatal: unable to read controls\n");
  _exit(111);
}
void die_alias()
{
  substdio_puts(subfderr,"qmail-pw2u: fatal: unable to find ");
  substdio_puts(subfderr,auto_usera);
  substdio_puts(subfderr," user\n");
  substdio_flush(subfderr);
  _exit(111);
}
void die_home(fn) char *fn;
{
  substdio_puts(subfderr,"qmail-pw2u: fatal: unable to stat ");
  substdio_puts(subfderr,fn);
  substdio_puts(subfderr,"\n");
  substdio_flush(subfderr);
  _exit(111);
}
void die_user(s,len) char *s; unsigned int len;
{
  substdio_puts(subfderr,"qmail-pw2u: fatal: unable to find ");
  substdio_put(subfderr,s,len);
  substdio_puts(subfderr," user for subuser\n");
  substdio_flush(subfderr);
  _exit(111);
}

char *dashcolon = "-:";
int flagalias = 0;
int flagnoupper = 1;
int homestrategy = 2;
/* 2: skip if home does not exist; skip if home is not owned by user */
/* 1: stop if home does not exist; skip if home is not owned by user */
/* 0: don't worry about home */

int okincl; stralloc incl = {0}; struct constmap mapincl;
int okexcl; stralloc excl = {0}; struct constmap mapexcl;
int okmana; stralloc mana = {0}; struct constmap mapmana;

stralloc allusers = {0}; struct constmap mapuser;

stralloc uugh = {0};
stralloc user = {0};
stralloc uidstr = {0};
stralloc gidstr = {0};
stralloc home = {0};
unsigned long uid;

stralloc line = {0};

void doaccount()
{
  struct stat st;
  int i;
  char *mailnames;
  char *x;
  unsigned int xlen;

  if (byte_chr(line.s,line.len,'\0') < line.len) return;

  x = line.s; xlen = line.len; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  if (!stralloc_copyb(&user,x,i)) die_nomem();
  if (!stralloc_0(&user)) die_nomem();
  ++i; x += i; xlen -= i; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  ++i; x += i; xlen -= i; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  if (!stralloc_copyb(&uidstr,x,i)) die_nomem();
  if (!stralloc_0(&uidstr)) die_nomem();
  scan_ulong(uidstr.s,&uid);
  ++i; x += i; xlen -= i; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  if (!stralloc_copyb(&gidstr,x,i)) die_nomem();
  if (!stralloc_0(&gidstr)) die_nomem();
  ++i; x += i; xlen -= i; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  ++i; x += i; xlen -= i; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  if (!stralloc_copyb(&home,x,i)) die_nomem();
  if (!stralloc_0(&home)) die_nomem();

  if (!uid) return;
  if (flagnoupper)
    for (i = 0;i < user.len;++i)
      if ((user.s[i] >= 'A') && (user.s[i] <= 'Z'))
	return;
  if (okincl)
    if (!constmap(&mapincl,user.s,user.len - 1))
      return;
  if (okexcl)
    if (constmap(&mapexcl,user.s,user.len - 1))
      return;
  if (homestrategy) {
    if (stat(home.s,&st) == -1) {
      if (errno != error_noent) die_home(home.s);
      if (homestrategy == 1) die_home(home.s);
      return;
    }
    if (st.st_uid != uid) return;
  }

  if (!stralloc_copys(&uugh,":")) die_nomem();
  if (!stralloc_cats(&uugh,user.s)) die_nomem();
  if (!stralloc_cats(&uugh,":")) die_nomem();
  if (!stralloc_cats(&uugh,uidstr.s)) die_nomem();
  if (!stralloc_cats(&uugh,":")) die_nomem();
  if (!stralloc_cats(&uugh,gidstr.s)) die_nomem();
  if (!stralloc_cats(&uugh,":")) die_nomem();
  if (!stralloc_cats(&uugh,home.s)) die_nomem();
  if (!stralloc_cats(&uugh,":")) die_nomem();

  /* XXX: avoid recording in allusers unless sub actually needs it */
  if (!stralloc_cats(&allusers,user.s)) die_nomem();
  if (!stralloc_cats(&allusers,":")) die_nomem();
  if (!stralloc_catb(&allusers,uugh.s,uugh.len)) die_nomem();
  if (!stralloc_0(&allusers)) die_nomem();

  if (str_equal(user.s,auto_usera)) {
    if (substdio_puts(subfdout,"+") == -1) die_write();
    if (substdio_put(subfdout,uugh.s,uugh.len) == -1) die_write();
    if (substdio_puts(subfdout,dashcolon) == -1) die_write();
    if (substdio_puts(subfdout,":\n") == -1) die_write();
    flagalias = 1;
  }

  mailnames = 0;
  if (okmana)
    mailnames = constmap(&mapmana,user.s,user.len - 1);
  if (!mailnames)
    mailnames = user.s;

  for (;;) {
    while (*mailnames == ':') ++mailnames;
    if (!*mailnames) break;

    i = str_chr(mailnames,':');

    if (substdio_puts(subfdout,"=") == -1) die_write();
    if (substdio_put(subfdout,mailnames,i) == -1) die_write();
    if (substdio_put(subfdout,uugh.s,uugh.len) == -1) die_write();
    if (substdio_puts(subfdout,"::\n") == -1) die_write();
  
    if (*auto_break) {
      if (substdio_puts(subfdout,"+") == -1) die_write();
      if (substdio_put(subfdout,mailnames,i) == -1) die_write();
      if (substdio_put(subfdout,auto_break,1) == -1) die_write();
      if (substdio_put(subfdout,uugh.s,uugh.len) == -1) die_write();
      if (substdio_puts(subfdout,dashcolon) == -1) die_write();
      if (substdio_puts(subfdout,":\n") == -1) die_write();
    }

    mailnames += i;
  }
}

stralloc sub = {0};

void dosubuser()
{
  int i;
  char *x;
  unsigned int xlen;
  char *uugh;

  x = line.s; xlen = line.len; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  if (!stralloc_copyb(&sub,x,i)) die_nomem();
  ++i; x += i; xlen -= i; i = byte_chr(x,xlen,':'); if (i == xlen) return;
  uugh = constmap(&mapuser,x,i);
  if (!uugh) die_user(x,i);
  ++i; x += i; xlen -= i; i = byte_chr(x,xlen,':'); if (i == xlen) return;

  if (substdio_puts(subfdout,"=") == -1) die_write();
  if (substdio_put(subfdout,sub.s,sub.len) == -1) die_write();
  if (substdio_puts(subfdout,uugh) == -1) die_write();
  if (substdio_puts(subfdout,dashcolon) == -1) die_write();
  if (substdio_put(subfdout,x,i) == -1) die_write();
  if (substdio_puts(subfdout,":\n") == -1) die_write();

  if (*auto_break) {
    if (substdio_puts(subfdout,"+") == -1) die_write();
    if (substdio_put(subfdout,sub.s,sub.len) == -1) die_write();
    if (substdio_put(subfdout,auto_break,1) == -1) die_write();
    if (substdio_puts(subfdout,uugh) == -1) die_write();
    if (substdio_puts(subfdout,dashcolon) == -1) die_write();
    if (substdio_put(subfdout,x,i) == -1) die_write();
    if (substdio_puts(subfdout,"-:\n") == -1) die_write();
  }
}

int fd;
substdio ss;
char ssbuf[SUBSTDIO_INSIZE];

void main(argc,argv)
int argc;
char **argv;
{
  int opt;
  int match;

  while ((opt = getopt(argc,argv,"/ohHuUc:C")) != opteof)
    switch(opt) {
      case '/': dashcolon = "-/:"; break;
      case 'o': homestrategy = 2; break;
      case 'h': homestrategy = 1; break;
      case 'H': homestrategy = 0; break;
      case 'u': flagnoupper = 0; break;
      case 'U': flagnoupper = 1; break;
      case 'c': *auto_break = *optarg; break;
      case 'C': *auto_break = 0; break;
      case '?':
      default:
	_exit(100);
    }

  if (chdir(auto_qmail) == -1) die_chdir();

  /* no need for control_init() */

  okincl = control_readfile(&incl,"users/include",0);
  if (okincl == -1) die_control();
  if (okincl) if (!constmap_init(&mapincl,incl.s,incl.len,0)) die_nomem();

  okexcl = control_readfile(&excl,"users/exclude",0);
  if (okexcl == -1) die_control();
  if (okexcl) if (!constmap_init(&mapexcl,excl.s,excl.len,0)) die_nomem();

  okmana = control_readfile(&mana,"users/mailnames",0);
  if (okmana == -1) die_control();
  if (okmana) if (!constmap_init(&mapmana,mana.s,mana.len,1)) die_nomem();

  if (!stralloc_copys(&allusers,"")) die_nomem();

  for (;;) {
    if (getln(subfdin,&line,&match,'\n') == -1) die_read();
    doaccount();
    if (!match) break;
  }
  if (!flagalias) die_alias();

  fd = open_read("users/subusers");
  if (fd == -1) {
    if (errno != error_noent) die_control();
  }
  else {
    substdio_fdbuf(&ss,read,fd,ssbuf,sizeof(ssbuf));

    if (!constmap_init(&mapuser,allusers.s,allusers.len,1)) die_nomem();

    for (;;) {
      if (getln(&ss,&line,&match,'\n') == -1) die_read();
      dosubuser();
      if (!match) break;
    }

    close(fd);
  }

  fd = open_read("users/append");
  if (fd == -1) {
    if (errno != error_noent) die_control();
  }
  else {
    substdio_fdbuf(&ss,read,fd,ssbuf,sizeof(ssbuf));
    for (;;) {
      if (getln(&ss,&line,&match,'\n') == -1) die_read();
      if (substdio_put(subfdout,line.s,line.len) == -1) die_write();
      if (!match) break;
    }
  }

  if (substdio_puts(subfdout,".\n") == -1) die_write();
  if (substdio_flush(subfdout) == -1) die_write();
  _exit(0);
}
