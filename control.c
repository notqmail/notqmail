#include "readwrite.h"
#include "open.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "error.h"
#include "control.h"
#include "alloc.h"
#include "scan.h"

static char inbuf[64];
static stralloc line = {0};
static stralloc me = {0};
static int meok = 0;

static void striptrailingwhitespace(sa)
stralloc *sa;
{
 while (sa->len > 0)
   switch(sa->s[sa->len - 1])
    {
     case '\n': case ' ': case '\t':
       --sa->len;
       break;
     default:
       return;
    }
}

int control_init()
{
 int r;
 r = control_readline(&me,"control/me");
 if (r == 1) meok = 1;
 return r;
}

int control_rldef(sa,fn,flagme,def)
stralloc *sa;
char *fn;
int flagme;
char *def;
{
 int r;
 r = control_readline(sa,fn);
 if (r) return r;
 if (flagme) if (meok) return stralloc_copy(sa,&me) ? 1 : -1;
 if (def) return stralloc_copys(sa,def) ? 1 : -1;
 return r;
}

int control_readline(sa,fn)
stralloc *sa;
char *fn;
{
 substdio ss;
 int fd;
 int match;

 fd = open_read(fn);
 if (fd == -1) { if (errno == error_noent) return 0; return -1; }
 
 substdio_fdbuf(&ss,read,fd,inbuf,sizeof(inbuf));

 if (getln(&ss,sa,&match,'\n') == -1) { close(fd); return -1; }

 striptrailingwhitespace(sa);
 close(fd);
 return 1;
}

int control_readint(i,fn)
int *i;
char *fn;
{
 unsigned long u;
 switch(control_readline(&line,fn))
  {
   case 0: return 0;
   case -1: return -1;
  }
 if (!stralloc_0(&line)) return -1;
 if (!scan_ulong(line.s,&u)) return 0;
 *i = u;
 return 1;
}

int control_readfile(sa,fn,flagme)
stralloc *sa;
char *fn;
int flagme;
{
 substdio ss;
 int fd;
 int match;

 if (!stralloc_copys(sa,"")) return -1;

 fd = open_read(fn);
 if (fd == -1) 
  {
   if (errno == error_noent)
    {
     if (flagme && meok)
      {
       if (!stralloc_copy(sa,&me)) return -1;
       if (!stralloc_0(sa)) return -1;
       return 1;
      }
     return 0;
    }
   return -1;
  }

 substdio_fdbuf(&ss,read,fd,inbuf,sizeof(inbuf));

 for (;;)
  {
   if (getln(&ss,&line,&match,'\n') == -1) break;
   if (!match && !line.len) { close(fd); return 1; }
   striptrailingwhitespace(&line);
   if (!stralloc_0(&line)) break;
   if (line.s[0])
     if (line.s[0] != '#')
       if (!stralloc_cat(sa,&line)) break;
   if (!match) { close(fd); return 1; }
  }
 close(fd);
 return -1;
}
