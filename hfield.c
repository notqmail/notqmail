#include "hfield.h"

static char *(hname[]) = {
  "unknown-header"
, "sender"
, "from"
, "reply-to"
, "to"
, "cc"
, "bcc"
, "date"
, "message-id"
, "subject"
, "resent-sender"
, "resent-from"
, "resent-reply-to"
, "resent-to"
, "resent-cc"
, "resent-bcc"
, "resent-date"
, "resent-message-id"
, "return-receipt-to"
, "errors-to"
, "apparently-to"
, "received"
, "return-path"
, "delivered-to"
, "content-length"
, "content-type"
, "content-transfer-encoding"
, "notice-requested-upon-delivery-to"
, "mail-followup-to"
, 0
};

static int hmatch(s,len,t)
char *s;
int len;
char *t;
{
 int i;
 char ch;

 for (i = 0;ch = t[i];++i)
  {
   if (i >= len) return 0;
   if (ch != s[i])
    {
     if (ch == '-') return 0;
     if (ch - 32 != s[i]) return 0;
    }
  }
 for (;;)
  {
   if (i >= len) return 0;
   ch = s[i];
   if (ch == ':') return 1;
   if ((ch != ' ') && (ch != '\t')) return 0;
   ++i;
  }
}

int hfield_known(s,len)
char *s;
int len;
{
 int i;
 char *t;

 for (i = 1;t = hname[i];++i)
   if (hmatch(s,len,t))
     return i;
 return 0;
}

int hfield_valid(s,len)
char *s;
int len;
{
 int i;
 int j;
 char ch;

 for (j = 0;j < len;++j)
   if (s[j] == ':')
     break;
 if (j >= len) return 0;
 while (j)
  {
   ch = s[j - 1];
   if ((ch != ' ') && (ch != '\t'))
     break;
   --j;
  }
 if (!j) return 0;

 for (i = 0;i < j;++i)
  {
   ch = s[i];
   if (ch <= 32) return 0;
   if (ch >= 127) return 0;
  }
 return 1;
}

unsigned int hfield_skipname(s,len)
char *s;
int len;
{
 int i;
 char ch;

 for (i = 0;i < len;++i)
   if (s[i] == ':')
     break;
 if (i < len) ++i;
 while (i < len)
  {
   ch = s[i];
   if ((ch != '\t') && (ch != '\n') && (ch != '\r') && (ch != ' '))
     break;
   ++i;
  }
 return i;
}
