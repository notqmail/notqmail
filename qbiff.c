#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#ifndef UTMP_FILE
#ifdef _PATH_UTMP
#define UTMP_FILE _PATH_UTMP
#else
#define UTMP_FILE "/etc/utmp"
#endif
#endif
#include "readwrite.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "open.h"
#include "byte.h"
#include "str.h"
#include "headerbody.h"
#include "hfield.h"
#include "env.h"
#include "exit.h"

substdio ssutmp;
char bufutmp[sizeof(struct utmp) * 16];
int fdutmp;
substdio sstty;
char buftty[1024];
int fdtty;

struct utmp ut;
char line[sizeof(ut.ut_line) + 1];
stralloc woof = {0};
stralloc tofrom = {0};
stralloc text = {0};

void doit(s,n) char *s; int n;
{
 if (!stralloc_catb(&text,s,n)) _exit(0);
 if (text.len > 78) text.len = 78;
}
void dobody(h) stralloc *h; { doit(h->s,h->len); }
void doheader(h) stralloc *h;
{
 int i;
 if (hfield_known(h->s,h->len) == H_SUBJECT)
  {
   i = hfield_skipname(h->s,h->len);
   doit(h->s + i,h->len - i);
  }
}
void finishheader() { ; }

void main()
{
 char *user;
 char *sender;
 char *userext;
 struct stat st;
 int i;

 if (chdir("/dev") == -1) _exit(0);

 if (!(user = env_get("USER"))) _exit(0);
 if (!(sender = env_get("SENDER"))) _exit(0);
 if (!(userext = env_get("LOCAL"))) _exit(0);
 if (str_len(user) > sizeof(ut.ut_name)) _exit(0);

 if (!stralloc_copys(&tofrom,"*** TO <")) _exit(0);
 if (!stralloc_cats(&tofrom,userext)) _exit(0);
 if (!stralloc_cats(&tofrom,"> FROM <")) _exit(0);
 if (!stralloc_cats(&tofrom,sender)) _exit(0);
 if (!stralloc_cats(&tofrom,">")) _exit(0);

 for (i = 0;i < tofrom.len;++i)
   if ((tofrom.s[i] < 32) || (tofrom.s[i] > 126))
     tofrom.s[i] = '_';

 if (!stralloc_copys(&text,"    ")) _exit(0);
 if (headerbody(subfdin,doheader,finishheader,dobody) == -1) _exit(0);

 for (i = 0;i < text.len;++i)
   if ((text.s[i] < 32) || (text.s[i] > 126))
     text.s[i] = '/';

 if (!stralloc_copys(&woof,"\015\n\007")) _exit(0);
 if (!stralloc_cat(&woof,&tofrom)) _exit(0);
 if (!stralloc_cats(&woof,"\015\n")) _exit(0);
 if (!stralloc_cat(&woof,&text)) _exit(0);
 if (!stralloc_cats(&woof,"\015\n")) _exit(0);

 fdutmp = open_read(UTMP_FILE);
 if (fdutmp == -1) _exit(0);
 substdio_fdbuf(&ssutmp,read,fdutmp,bufutmp,sizeof(bufutmp));

 while (substdio_get(&ssutmp,&ut,sizeof(ut)) == sizeof(ut))
   if (!str_diffn(ut.ut_name,user,sizeof(ut.ut_name)))
    {
     byte_copy(line,sizeof(ut.ut_line),ut.ut_line);
     line[sizeof(ut.ut_line)] = 0;
     if (line[0] == '/') continue;
     if (!line[0]) continue;
     if (line[str_chr(line,'.')]) continue;
     fdtty = open_append(line);
     if (fdtty == -1) continue;
     if (fstat(fdtty,&st) == -1) { close(fdtty); continue; }
     if (!(st.st_mode & 0100)) { close(fdtty); continue; }
     if (st.st_uid != getuid()) { close(fdtty); continue; }
     substdio_fdbuf(&sstty,write,fdtty,buftty,sizeof(buftty));
     substdio_putflush(&sstty,woof.s,woof.len);
     close(fdtty);
    }
 _exit(0);
}
