#include "getln.h"
#include "substdio.h"
#include "subfd.h"
#include "prioq.h"
#include "stralloc.h"
#include "str.h"
#include "exit.h"
#include "hfield.h"
#include "readwrite.h"
#include "open.h"
#include "headerbody.h"
#include "maildir.h"

#define FATAL "maildirwatch: fatal: "

void die_nomem() { strerr_die2x(111,FATAL,"out of memory"); }

stralloc recipient = {0};
stralloc sender = {0};
stralloc fromline = {0};
stralloc text = {0};

void addtext(s,n) char *s; int n;
{
 if (!stralloc_catb(&text,s,n)) die_nomem();
 if (text.len > 158) text.len = 158;
}
void dobody(h) stralloc *h; { addtext(h->s,h->len); }
void doheader(h) stralloc *h;
{
 int i;
 switch(hfield_known(h->s,h->len))
  {
   case H_SUBJECT:
     i = hfield_skipname(h->s,h->len);
     addtext(h->s + i,h->len - i);
     break;
   case H_DELIVEREDTO:
     i = hfield_skipname(h->s,h->len);
     if (i < h->len)
       if (!stralloc_copyb(&recipient,h->s + i,h->len - i - 1)) die_nomem();
     break;
   case H_RETURNPATH:
     i = hfield_skipname(h->s,h->len);
     if (i < h->len)
       if (!stralloc_copyb(&sender,h->s + i,h->len - i - 1)) die_nomem();
     break;
   case H_FROM:
     if (!stralloc_copyb(&fromline,h->s,h->len - 1)) die_nomem();
     break;
  }
}
void finishheader() { ; }

stralloc filenames = {0};
prioq pq = {0};

char inbuf[SUBSTDIO_INSIZE];
substdio ssin;

void main()
{
 struct prioq_elt pe;
 int fd;
 int i;

 if (maildir_chdir() == -1)
   strerr_die1(111,FATAL,&maildir_chdir_err);

 for (;;)
  {
   maildir_clean(&filenames);
   if (maildir_scan(&pq,&filenames,1,0) == -1)
     strerr_die1(111,FATAL,&maildir_scan_err);

   substdio_putsflush(subfdout,"\033[;H\033[;J");

   while (prioq_min(&pq,&pe))
    {
     prioq_delmin(&pq);

     fd = open_read(filenames.s + pe.id);
     if (fd == -1) continue;
     substdio_fdbuf(&ssin,read,fd,inbuf,sizeof(inbuf));

     if (!stralloc_copys(&sender,"?")) die_nomem();
     if (!stralloc_copys(&recipient,"?")) die_nomem();
     if (!stralloc_copys(&fromline,"")) die_nomem();
     if (!stralloc_copys(&text,"")) die_nomem();
     if (headerbody(&ssin,doheader,finishheader,dobody) == -1)
       strerr_die2x(111,FATAL,"trouble reading new message");

     for (i = 0;i < fromline.len;++i)
       if ((fromline.s[i] < 32) || (fromline.s[i] > 126))
         fromline.s[i] = '/';
     for (i = 0;i < sender.len;++i)
       if ((sender.s[i] < 32) || (sender.s[i] > 126))
         sender.s[i] = '?';
     for (i = 0;i < recipient.len;++i)
       if ((recipient.s[i] < 32) || (recipient.s[i] > 126))
         recipient.s[i] = '?';
     for (i = 0;i < text.len;++i)
       if ((text.s[i] < 32) || (text.s[i] > 126))
         text.s[i] = '/';
     substdio_puts(subfdout,"FROM ");
     substdio_put(subfdout,sender.s,sender.len);
     substdio_puts(subfdout," TO <");
     substdio_put(subfdout,recipient.s,recipient.len);
     substdio_puts(subfdout,">\n");
     if (fromline.len)
      {
       substdio_puts(subfdout,"\033[1m");
       substdio_put(subfdout,fromline.s,fromline.len);
       substdio_puts(subfdout,"\033[0m\n");
      }
     substdio_put(subfdout,text.s,text.len);
     substdio_puts(subfdout,"\n\n");

     close(fd);
    }

   substdio_flush(subfdout);
   sleep(30);
  }
}
