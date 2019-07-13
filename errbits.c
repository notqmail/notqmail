#include "stralloc.h"
#include "readwrite.h"
#include "errbits.h"
#include "substdio.h"
#include "fmt.h"
#include "exit.h"

char sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write,2,sserrbuf,sizeof sserrbuf);
static stralloc foo = {0};

static char pid_str[FMT_ULONG]="?PID?";

void esetfd(fd) int fd; { sserr.fd=fd; }

void eout(s1)  char *s1; {substdio_puts(&sserr,s1);}
void eout2(s1,s2) char *s1,*s2; {substdio_puts(&sserr,s1);substdio_puts(&sserr,s2);}
void eout3(s1,s2,s3) char *s1,*s2,*s3; {substdio_puts(&sserr,s1);substdio_puts(&sserr,s2);substdio_puts(&sserr,s3);}

void epid()
{
  if (*pid_str == '?') /* not yet set from getpid() */
    pid_str[fmt_ulong(pid_str,getpid())] = 0;
  eout(pid_str);
}
void eflush() { substdio_flush(&sserr); }

/* The functions below here come from qsutil.c with minor changes */
void eoutsa(sa) stralloc *sa; { substdio_putflush(&sserr,sa->s,sa->len); }

static void nomem() { substdio_putsflush(&sserr,"Out Of Memory: quitting.\n"); _exit(1); }

static int issafe(ch) char ch;
{ /* Differs from qsutil.c version: space and % permitted */
 if (ch == ':') return 0; /* Replace since used as delimiter in logs */
 if (ch == '<') return 0; /* Replace since used around addresses in logs */
 if (ch == '>') return 0; /* Replace since used around addresses in logs */
 if (ch < 32)   return 0; /* Note that space (32) is permitted */
 if (ch > 126)  return 0;
 return 1;
}

void eoutclean(s) char *s;
{
 int i;
 while (!stralloc_copys(&foo,s)) nomem();
 for (i = 0;i < foo.len;++i)
   if (foo.s[i] == '\n')
     foo.s[i] = '/';
   else
     if (!issafe(foo.s[i]))
       foo.s[i] = '_';
 eoutsa(&foo);
}

static char ulongstr[FMT_ULONG];
void eoutulong(u) unsigned long u;
{
  ulongstr[fmt_ulong(ulongstr,u)] = 0;
  eout(ulongstr);
}

