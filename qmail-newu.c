#include "stralloc.h"
#include "subfd.h"
#include "getln.h"
#include "substdio.h"
#include "cdbmss.h"
#include "exit.h"
#include "readwrite.h"
#include "open.h"
#include "error.h"
#include "case.h"
#include "auto_qmail.h"

void die_temp() { _exit(111); }

void die_chdir()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: unable to chdir\n");
  die_temp();
}
void die_nomem()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: out of memory\n");
  die_temp();
}
void die_opena()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: unable to open users/assign\n");
  die_temp();
}
void die_reada()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: unable to read users/assign\n");
  die_temp();
}
void die_format()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: bad format in users/assign\n");
  die_temp();
}
void die_opent()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: unable to open users/cdb.tmp\n");
  die_temp();
}
void die_writet()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: unable to write users/cdb.tmp\n");
  die_temp();
}
void die_rename()
{
  substdio_putsflush(subfderr,"qmail-newu: fatal: unable to move users/cdb.tmp to users/cdb\n");
  die_temp();
}

struct cdbmss cdbmss;
stralloc key = {0};
stralloc data = {0};

char inbuf[1024];
substdio ssin;

int fd;
int fdtemp;

stralloc line = {0};
int match;

stralloc wildchars = {0};

void main()
{
  int i;
  int numcolons;

  umask(033);
  if (chdir(auto_qmail) == -1) die_chdir();

  fd = open_read("users/assign");
  if (fd == -1) die_opena();

  substdio_fdbuf(&ssin,read,fd,inbuf,sizeof(inbuf));

  fdtemp = open_trunc("users/cdb.tmp");
  if (fdtemp == -1) die_opent();

  if (cdbmss_start(&cdbmss,fdtemp) == -1) die_writet();

  if (!stralloc_copys(&wildchars,"")) die_nomem();

  for (;;) {
    if (getln(&ssin,&line,&match,'\n') != 0) die_reada();
    if (line.len && (line.s[0] == '.')) break;
    if (!match) die_format();

    if (byte_chr(line.s,line.len,'\0') < line.len) die_format();
    i = byte_chr(line.s,line.len,':');
    if (i == line.len) die_format();
    if (i == 0) die_format();
    if (!stralloc_copys(&key,"!")) die_nomem();
    if (line.s[0] == '+') {
      if (!stralloc_catb(&key,line.s + 1,i - 1)) die_nomem();
      case_lowerb(key.s,key.len);
      if (i >= 2)
	if (byte_chr(wildchars.s,wildchars.len,line.s[i - 1]) == wildchars.len)
	  if (!stralloc_append(&wildchars,line.s + i - 1)) die_nomem();
    }
    else {
      if (!stralloc_catb(&key,line.s + 1,i - 1)) die_nomem();
      if (!stralloc_0(&key)) die_nomem();
      case_lowerb(key.s,key.len);
    }

    if (!stralloc_copyb(&data,line.s + i + 1,line.len - i - 1)) die_nomem();

    numcolons = 0;
    for (i = 0;i < data.len;++i)
      if (data.s[i] == ':') {
	data.s[i] = 0;
	if (++numcolons == 6)
	  break;
      }
    if (numcolons < 6) die_format();
    data.len = i;

    if (cdbmss_add(&cdbmss,key.s,key.len,data.s,data.len) == -1) die_writet();
  }

  if (cdbmss_add(&cdbmss,"",0,wildchars.s,wildchars.len) == -1) die_writet();

  if (cdbmss_finish(&cdbmss) == -1) die_writet();
  if (fsync(fdtemp) == -1) die_writet();
  if (close(fdtemp) == -1) die_writet(); /* NFS stupidity */
  if (rename("users/cdb.tmp","users/cdb") == -1) die_rename();

  _exit(0);
}
