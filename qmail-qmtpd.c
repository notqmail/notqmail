#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "qmail.h"
#include "now.h"
#include "str.h"
#include "fmt.h"
#include "env.h"
#include "sig.h"
#include "auto_qmail.h"
#include "now.h"
#include "datetime.h"
#include "date822fmt.h"
#include "readwrite.h"
#include "control.h"
#include "constmap.h"
#include "received.h"

struct qmail qqt;

void dropped() { _exit(0); }
void badproto() { _exit(100); }
void resources() { _exit(111); }
void sigalrm() { _exit(111); }

unsigned long getlen()
{
 unsigned long len;
 char ch;

 len = 0;
 for (;;)
  {
   if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
   if (ch == ':') return len;
   if (len > 200000000) resources();
   len = 10 * len + (ch - '0');
  }
}

void getcomma()
{
 char ch;

 if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
 if (ch != ',') badproto();
}

struct datetime dt;
char buf[1000];
char buf2[100];

char *remotehost;
char *remoteinfo;
char *remoteip;
char *local;

stralloc failure = {0};

int flagrcpthosts;
stralloc rcpthosts = {0};
struct constmap maprcpthosts;
char *relayclient;
int relayclientlen;

int addrallowed(buf,len) char *buf; int len;
{
 int j;
 if (!flagrcpthosts) return 1;
 j = byte_rchr(buf,len,'@');
 if (j >= len) return 1;
 if (constmap(&maprcpthosts,buf + j + 1,len - j - 1)) return 1;
 for (;j < len;++j)
   if (buf[j] == '.')
     if (constmap(&maprcpthosts,buf + j,len - j)) return 1;
 return 0;
}

main()
{
 char ch;
 int i;
 unsigned long biglen;
 unsigned long len;
 int flagdos;
 int flagsenderok;
 unsigned long qp;
 char *result;

 sig_pipeignore();
 sig_alarmcatch(sigalrm);
 alarm(3600);

 if (chdir(auto_qmail) == -1) resources();

 if (control_init() == -1) resources();
 flagrcpthosts = control_readfile(&rcpthosts,"control/rcpthosts",0);
 if (flagrcpthosts == -1) resources();
 if (flagrcpthosts)
   if (!constmap_init(&maprcpthosts,rcpthosts.s,rcpthosts.len,0)) resources();
 relayclient = env_get("RELAYCLIENT");
 relayclientlen = relayclient ? str_len(relayclient) : 0;

 remotehost = env_get("TCPREMOTEHOST");
 if (!remotehost) remotehost = "unknown";
 remoteinfo = env_get("TCPREMOTEINFO");
 remoteip = env_get("TCPREMOTEIP");
 if (!remoteip) remoteip = "unknown";
 local = env_get("TCPLOCALHOST");
 if (!local) local = env_get("TCPLOCALIP");
 if (!local) local = "unknown";

 for (;;)
  {
   if (!stralloc_copys(&failure,"")) resources();
   flagsenderok = 1;

   len = getlen();
   if (len == 0) badproto();

   if (qmail_open(&qqt) == -1) resources();
   qp = qmail_qp(&qqt);

   if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
   --len;

   if (ch == 10) flagdos = 0;
   else if (ch == 13) flagdos = 1;
   else badproto();

   received(&qqt,"QMTP",local,remoteip,remotehost,remoteinfo,(char *) 0);

   /* XXX: check for loops? only if len is big? */

   if (flagdos)
     while (len > 0)
      {
       if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
       --len;
       while ((ch == 13) && len)
        {
         if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
         --len;
         if (ch == 10) break;
         qmail_put(&qqt,"\015",1);
        }
       qmail_put(&qqt,&ch,1);
      }
   else
     while (len > 0) /* XXX: could speed this up, obviously */
      {
       if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
       --len;
       qmail_put(&qqt,&ch,1);
      }
   getcomma();

   len = getlen();

   if (len >= 1000)
    {
     buf[0] = 0;
     flagsenderok = 0;
     for (i = 0;i < len;++i)
       if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
    }
   else
    {
     for (i = 0;i < len;++i)
      {
       if (substdio_get(subfdinsmall,buf + i,1) < 1) dropped();
       if (!buf[i]) flagsenderok = 0;
      }
     buf[len] = 0;
    }
   getcomma();

   qmail_from(&qqt,buf);
   if (!flagsenderok) qmail_fail(&qqt);

   biglen = getlen();
   while (biglen > 0)
    {
     if (!stralloc_append(&failure,"")) resources();

     len = 0;
     for (;;)
      {
       if (!biglen) badproto();
       if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
       --biglen;
       if (ch == ':') break;
       if (len > 200000000) resources();
       len = 10 * len + (ch - '0');
      }
     if (len >= biglen) badproto();
     if (len + relayclientlen >= 1000)
      {
       failure.s[failure.len - 1] = 'L';
       for (i = 0;i < len;++i)
         if (substdio_get(subfdinsmall,&ch,1) < 1) dropped();
      }
     else
      {
       for (i = 0;i < len;++i)
        {
         if (substdio_get(subfdinsmall,buf + i,1) < 1) dropped();
         if (!buf[i]) failure.s[failure.len - 1] = 'N';
        }
       buf[len] = 0;

       if (relayclient)
	 str_copy(buf + len,relayclient);
       else
	 if (!addrallowed(buf,len)) failure.s[failure.len - 1] = 'D';

       if (!failure.s[failure.len - 1])
	 qmail_to(&qqt,buf);
      }
     getcomma();
     biglen -= (len + 1);
    }
   getcomma();

   switch(qmail_close(&qqt))
    {
     case 0: result = 0; break;
     case QMAIL_WAITPID: result = "Zqq waitpid surprise (#4.3.0)"; break;
     case QMAIL_CRASHED: result = "Zqq crashed (#4.3.0)"; break;
     case QMAIL_USAGE: result = "Zqq usage surprise (#4.3.0)"; break;
     case QMAIL_SYS: result = "Zqq system error (#4.3.0)"; break;
     case QMAIL_READ: result = "Zqq read error (#4.3.0)"; break;
     case QMAIL_WRITE: result = "Zqq write error or disk full (#4.3.0)"; break;
     case QMAIL_NOMEM: result = "Zqq out of memory (#4.3.0)"; break;
     case QMAIL_EXECSOFT: result = "Zcould not exec qq (#4.3.0)"; break;
     case QMAIL_TIMEOUT: result = "Zqq timeout (#4.3.0)"; break;
     case QMAIL_TOOLONG: result = "Dqq toolong surprise (#5.1.3)"; break;
     default: result = "Zqq internal bug (#4.3.0)"; break;
    }

   if (!flagsenderok) result = "Dunacceptable sender (#5.1.7)";

   if (result)
     len = str_len(result);
   else
    {
     /* success! */
     len = 0;
     len += fmt_str(buf2 + len,"Kok ");
     len += fmt_ulong(buf2 + len,(unsigned long) now());
     len += fmt_str(buf2 + len," qp ");
     len += fmt_ulong(buf2 + len,qp);
     buf2[len] = 0;
     result = buf2;
    }
     
   len = fmt_ulong(buf,len);
   buf[len++] = ':';
   len += fmt_str(buf + len,result);
   buf[len++] = ',';

   for (i = 0;i < failure.len;++i)
     switch(failure.s[i])
      {
       case 0:
         if (substdio_put(subfdoutsmall,buf,len) == -1)
           dropped();
	 break;
       case 'D':
         if (substdio_puts(subfdoutsmall,"66:Dsorry, that domain isn't in my list of allowed rcpthosts (#5.7.1),") == -1)
	   dropped();
	 break;
       default:
         if (substdio_puts(subfdoutsmall,"46:Dsorry, I can't handle that recipient (#5.1.3),") == -1)
	   dropped();
	 break;
      }

   /* subfdoutsmall will be flushed when we read from the network again */
  }
}
