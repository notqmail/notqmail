#include "fmt.h"
#include "datetime.h"
#include "stralloc.h"
#include "date822fmt.h"
#include "newfield.h"

/* "Date: 26 Sep 1995 04:46:53 -0000\n" */
stralloc newfield_date = {0};
/* "Message-ID: <19950926044653.12345.qmail@silverton.berkeley.edu>\n" */
stralloc newfield_msgid = {0};

static unsigned int datefmt(s,when)
char *s;
datetime_sec when;
{
 unsigned int i;
 unsigned int len;
 struct datetime dt;
 datetime_tai(&dt,when);
 len = 0;
 i = fmt_str(s,"Date: "); len += i; if (s) s += i;
 i = date822fmt(s,&dt); len += i; if (s) s += i;
 return len;
}

static unsigned int msgidfmt(s,idhost,idhostlen,when)
char *s;
char *idhost;
int idhostlen;
datetime_sec when;
{
 unsigned int i;
 unsigned int len;
 struct datetime dt;
 datetime_tai(&dt,when);
 len = 0;
 i = fmt_str(s,"Message-ID: <"); len += i; if (s) s += i;
 i = fmt_uint(s,dt.year + 1900); len += i; if (s) s += i;
 i = fmt_uint0(s,dt.mon + 1,2); len += i; if (s) s += i;
 i = fmt_uint0(s,dt.mday,2); len += i; if (s) s += i;
 i = fmt_uint0(s,dt.hour,2); len += i; if (s) s += i;
 i = fmt_uint0(s,dt.min,2); len += i; if (s) s += i;
 i = fmt_uint0(s,dt.sec,2); len += i; if (s) s += i;
 i = fmt_str(s,"."); len += i; if (s) s += i;
 i = fmt_uint(s,getpid()); len += i; if (s) s += i;
 i = fmt_str(s,".qmail@"); len += i; if (s) s += i;
 i = fmt_strn(s,idhost,idhostlen); len += i; if (s) s += i;
 i = fmt_str(s,">\n"); len += i; if (s) s += i;
 return len;
}

int newfield_datemake(when)
datetime_sec when;
{
 if (!stralloc_ready(&newfield_date,datefmt(FMT_LEN,when))) return 0;
 newfield_date.len = datefmt(newfield_date.s,when);
 return 1;
}

int newfield_msgidmake(idhost,idhostlen,when)
char *idhost;
int idhostlen;
datetime_sec when;
{
 if (!stralloc_ready(&newfield_msgid,msgidfmt(FMT_LEN,idhost,idhostlen,when))) return 0;
 newfield_msgid.len = msgidfmt(newfield_msgid.s,idhost,idhostlen,when);
 return 1;
}
