#include "stralloc.h"
#include "alloc.h"
#include "str.h"
#include "token822.h"
#include "gen_allocdefs.h"

static struct token822 comma = { TOKEN822_COMMA };

void token822_reverse(ta)
token822_alloc *ta;
{
 int i;
 int n;
 struct token822 temp;

 n = ta->len - 1;
 for (i = 0;i + i < n;++i)
  {
   temp = ta->t[i];
   ta->t[i] = ta->t[n - i];
   ta->t[n - i] = temp;
  }
}

GEN_ALLOC_ready(token822_alloc,struct token822,t,len,a,i,n,x,30,token822_ready)
GEN_ALLOC_readyplus(token822_alloc,struct token822,t,len,a,i,n,x,30,token822_readyplus)
GEN_ALLOC_append(token822_alloc,struct token822,t,len,a,i,n,x,30,token822_readyplus,token822_append)

static int needspace(t1,t2)
int t1;
int t2;
{
 if (!t1) return 0;
 if (t1 == TOKEN822_COLON) return 1;
 if (t1 == TOKEN822_COMMA) return 1;
 if (t2 == TOKEN822_LEFT) return 1;
 switch(t1)
  {
   case TOKEN822_ATOM: case TOKEN822_LITERAL:
   case TOKEN822_QUOTE: case TOKEN822_COMMENT:
     switch(t2)
      {
       case TOKEN822_ATOM: case TOKEN822_LITERAL:
       case TOKEN822_QUOTE: case TOKEN822_COMMENT:
         return 1;
      }
  }
 return 0;
}

static int atomok(ch)
char ch;
{
 switch(ch)
  {
   case ' ': case '\t': case '\r': case '\n':
   case '(': case '[': case '"':
   case '<': case '>': case ';': case ':':
   case '@': case ',': case '.':
     return 0;
  }
 return 1;
}

static void atomcheck(t)
struct token822 *t;
{
 int i;
 char ch;
 for (i = 0;i < t->slen;++i)
  {
   ch = t->s[i];
   if ((ch < 32) || (ch > 126) || (ch == ')') || (ch == ']') || (ch == '\\'))
    {
     t->type = TOKEN822_QUOTE;
     return;
    }
  }
}

int token822_unparse(sa,ta,linelen)
stralloc *sa;
token822_alloc *ta;
unsigned int linelen;
{
 struct token822 *t;
 int len;
 int ch;
 int i;
 int j;
 int lasttype;
 int newtype;
 char *s;
 char *lineb;
 char *linee;

 len = 0;
 lasttype = 0;
 for (i = 0;i < ta->len;++i)
  {
   t = ta->t + i;
   newtype = t->type;
   if (needspace(lasttype,newtype))
     ++len;
   lasttype = newtype;
   switch(newtype)
    {
     case TOKEN822_COMMA:
       len += 3; break;
     case TOKEN822_AT: case TOKEN822_DOT: case TOKEN822_LEFT: case TOKEN822_RIGHT:
     case TOKEN822_SEMI: case TOKEN822_COLON:
       ++len; break;
     case TOKEN822_ATOM: case TOKEN822_QUOTE: case TOKEN822_LITERAL: case TOKEN822_COMMENT:
       if (t->type != TOKEN822_ATOM) len += 2;
       for (j = 0;j < t->slen;++j)
	 switch(ch = t->s[j])
	  {
	   case '"': case '[': case ']': case '(': case ')':
	   case '\\': case '\r': case '\n': ++len;
	   default: ++len;
	  }
       break;
    }
  }
 len += 2;

 if (!stralloc_ready(sa,len))
   return -1;

 s = sa->s;
 lineb = s;
 linee = 0;

 lasttype = 0;
 for (i = 0;i < ta->len;++i)
  {
   t = ta->t + i;
   newtype = t->type;
   if (needspace(lasttype,newtype))
     *s++ = ' ';
   lasttype = newtype;
   switch(newtype)
    {
     case TOKEN822_COMMA:
       *s++ = ',';
#define NSUW \
 s[0] = '\n'; s[1] = ' '; \
 if (linee && (!linelen || (s - lineb <= linelen))) \
  { while (linee < s) { linee[0] = linee[2]; ++linee; } linee -= 2; } \
 else { if (linee) lineb = linee + 1; linee = s; s += 2; }
       NSUW
       break;
     case TOKEN822_AT: *s++ = '@'; break;
     case TOKEN822_DOT: *s++ = '.'; break;
     case TOKEN822_LEFT: *s++ = '<'; break;
     case TOKEN822_RIGHT: *s++ = '>'; break;
     case TOKEN822_SEMI: *s++ = ';'; break;
     case TOKEN822_COLON: *s++ = ':'; break;
     case TOKEN822_ATOM: case TOKEN822_QUOTE: case TOKEN822_LITERAL: case TOKEN822_COMMENT:
       if (t->type == TOKEN822_QUOTE) *s++ = '"';
       if (t->type == TOKEN822_LITERAL) *s++ = '[';
       if (t->type == TOKEN822_COMMENT) *s++ = '(';
       for (j = 0;j < t->slen;++j)
	 switch(ch = t->s[j])
	  {
	   case '"': case '[': case ']': case '(': case ')':
	   case '\\': case '\r': case '\n': *s++ = '\\';
	   default: *s++ = ch;
	  }
       if (t->type == TOKEN822_QUOTE) *s++ = '"';
       if (t->type == TOKEN822_LITERAL) *s++ = ']';
       if (t->type == TOKEN822_COMMENT) *s++ = ')';
       break;
    }
  }
 NSUW
 --s;
 sa->len = s - sa->s;
 return 1;
}

int token822_unquote(sa,ta)
stralloc *sa;
token822_alloc *ta;
{
 struct token822 *t;
 int len;
 int i;
 int j;
 char *s;

 len = 0;
 for (i = 0;i < ta->len;++i)
  {
   t = ta->t + i;
   switch(t->type)
    {
     case TOKEN822_COMMA: case TOKEN822_AT: case TOKEN822_DOT: case TOKEN822_LEFT: 
     case TOKEN822_RIGHT: case TOKEN822_SEMI: case TOKEN822_COLON: 
       ++len; break;
     case TOKEN822_LITERAL:
       len += 2;
     case TOKEN822_ATOM: case TOKEN822_QUOTE:
       len += t->slen;
    }
  }

 if (!stralloc_ready(sa,len))
   return -1;

 s = sa->s;

 for (i = 0;i < ta->len;++i)
  {
   t = ta->t + i;
   switch(t->type)
    {
     case TOKEN822_COMMA: *s++ = ','; break;
     case TOKEN822_AT: *s++ = '@'; break;
     case TOKEN822_DOT: *s++ = '.'; break;
     case TOKEN822_LEFT: *s++ = '<'; break;
     case TOKEN822_RIGHT: *s++ = '>'; break;
     case TOKEN822_SEMI: *s++ = ';'; break;
     case TOKEN822_COLON: *s++ = ':'; break;
     case TOKEN822_ATOM: case TOKEN822_QUOTE: case TOKEN822_LITERAL:
       if (t->type == TOKEN822_LITERAL) *s++ = '[';
       for (j = 0;j < t->slen;++j)
	 *s++ = t->s[j];
       if (t->type == TOKEN822_LITERAL) *s++ = ']';
       break;
     case TOKEN822_COMMENT: break;
    }
  }
 sa->len = s - sa->s;
 return 1;
}

int token822_parse(ta,sa,buf)
token822_alloc *ta;
stralloc *sa;
stralloc *buf;
{
 int i;
 int salen;
 int level;
 struct token822 *t;
 int numtoks;
 int numchars;
 char *cbuf;

 salen = sa->len;

 numchars = 0;
 numtoks = 0;
 for (i = 0;i < salen;++i)
   switch(sa->s[i])
    {
     case '.': case ',': case '@': case '<': case '>': case ':': case ';':
       ++numtoks; break;
     case ' ': case '\t': case '\r': case '\n': break;
     case ')': case ']': return 0;
     /* other control chars and non-ASCII chars are also bad, in theory */
     case '(':
       level = 1;
       while (level)
	{
	 if (++i >= salen) return 0;
	 switch(sa->s[i])
	  {
	   case '(': ++level; break;
	   case ')': --level; break;
	   case '\\': if (++i >= salen) return 0;
	   default: ++numchars;
	  }
	}
       ++numtoks;
       break;
     case '"':
       level = 1;
       while (level)
	{
	 if (++i >= salen) return 0;
	 switch(sa->s[i])
	  {
	   case '"': --level; break;
	   case '\\': if (++i >= salen) return 0;
	   default: ++numchars;
	  }
	}
       ++numtoks;
       break;
     case '[':
       level = 1;
       while (level)
	{
	 if (++i >= salen) return 0;
	 switch(sa->s[i])
	  {
	   case ']': --level; break;
	   case '\\': if (++i >= salen) return 0;
	   default: ++numchars;
	  }
	}
       ++numtoks;
       break;
     default:
       do
	{
	 if (sa->s[i] == '\\') if (++i >= salen) break;
	 ++numchars;
	 if (++i >= salen)
	   break;
	}
       while (atomok(sa->s[i]));
       --i;
       ++numtoks;
    }

 if (!token822_ready(ta,numtoks))
   return -1;
 if (!stralloc_ready(buf,numchars))
   return -1;
 cbuf = buf->s;
 ta->len = numtoks;

 t = ta->t;
 for (i = 0;i < salen;++i)
   switch(sa->s[i])
    {
     case '.': t->type = TOKEN822_DOT; ++t; break;
     case ',': t->type = TOKEN822_COMMA; ++t; break;
     case '@': t->type = TOKEN822_AT; ++t; break;
     case '<': t->type = TOKEN822_LEFT; ++t; break;
     case '>': t->type = TOKEN822_RIGHT; ++t; break;
     case ':': t->type = TOKEN822_COLON; ++t; break;
     case ';': t->type = TOKEN822_SEMI; ++t; break;
     case ' ': case '\t': case '\r': case '\n': break;
     case '(':
       t->type = TOKEN822_COMMENT; t->s = cbuf; t->slen = 0;
       level = 1;
       while (level)
	{
	 ++i; /* assert: < salen */
	 switch(sa->s[i])
	  {
	   case '(': ++level; break;
	   case ')': --level; break;
	   case '\\': ++i; /* assert: < salen */
	   default: *cbuf++ = sa->s[i]; ++t->slen;
	  }
	}
       ++t;
       break;
     case '"':
       t->type = TOKEN822_QUOTE; t->s = cbuf; t->slen = 0;
       level = 1;
       while (level)
	{
	 ++i; /* assert: < salen */
	 switch(sa->s[i])
	  {
	   case '"': --level; break;
	   case '\\': ++i; /* assert: < salen */
	   default: *cbuf++ = sa->s[i]; ++t->slen;
	  }
	}
       ++t;
       break;
     case '[':
       t->type = TOKEN822_LITERAL; t->s = cbuf; t->slen = 0;
       level = 1;
       while (level)
	{
	 ++i; /* assert: < salen */
	 switch(sa->s[i])
	  {
	   case ']': --level; break;
	   case '\\': ++i; /* assert: < salen */
	   default: *cbuf++ = sa->s[i]; ++t->slen;
	  }
	}
       ++t;
       break;
     default:
       t->type = TOKEN822_ATOM; t->s = cbuf; t->slen = 0;
       do
	{
	 if (sa->s[i] == '\\') if (++i >= salen) break;
	 *cbuf++ = sa->s[i]; ++t->slen;
	 if (++i >= salen)
	   break;
	}
       while (atomok(sa->s[i]));
       atomcheck(t);
       --i;
       ++t;
    }
 return 1;
}

static int gotaddr(taout,taaddr,callback)
token822_alloc *taout;
token822_alloc *taaddr;
int (*callback)();
{
 int i;

 if (callback(taaddr) != 1)
   return 0;

 if (!token822_readyplus(taout,taaddr->len))
   return 0;
 
 for (i = 0;i < taaddr->len;++i)
   taout->t[taout->len++] = taaddr->t[i];

 taaddr->len = 0;
 return 1;
}

int token822_addrlist(taout,taaddr,ta,callback)
token822_alloc *taout;
token822_alloc *taaddr;
token822_alloc *ta;
int (*callback)();
{
 struct token822 *t;
 struct token822 *beginning;
 int ingroup;
 int wordok;

 taout->len = 0;
 taaddr->len = 0;

 if (!token822_readyplus(taout,1)) return -1;
 if (!token822_readyplus(taaddr,1)) return -1;
 
 ingroup = 0;
 wordok = 1;

 beginning = ta->t + 2;
 t = ta->t + ta->len - 1;

 /* rfc 822 address lists are easy to parse from right to left */

#define FLUSH if (taaddr->len) if (!gotaddr(taout,taaddr,callback)) return -1;
#define FLUSHCOMMA if (taaddr->len) { \
if (!gotaddr(taout,taaddr,callback)) return -1; \
if (!token822_append(taout,&comma)) return -1; }
#define ADDRLEFT if (!token822_append(taaddr,t--)) return -1;
#define OUTLEFT if (!token822_append(taout,t--)) return -1;

 while (t >= beginning)
  {
   switch(t->type)
    {
     case TOKEN822_SEMI:
       FLUSHCOMMA
       if (ingroup) return 0;
       ingroup = 1;
       wordok = 1;
       break;
     case TOKEN822_COLON:
       FLUSH
       if (!ingroup) return 0;
       ingroup = 0;
       while ((t >= beginning) && (t->type != TOKEN822_COMMA))
	 OUTLEFT
       if (t >= beginning)
	 OUTLEFT
       wordok = 1;
       continue;
     case TOKEN822_RIGHT:
       FLUSHCOMMA
       OUTLEFT
       while ((t >= beginning) && (t->type != TOKEN822_LEFT))
	 ADDRLEFT
       /* important to use address here even if it's empty: <> */
       if (!gotaddr(taout,taaddr,callback)) return -1;
       if (t < beginning) return 0;
       OUTLEFT
       while ((t >= beginning) && ((t->type == TOKEN822_COMMENT) || (t->type == TOKEN822_ATOM) || (t->type == TOKEN822_QUOTE) || (t->type == TOKEN822_AT) || (t->type == TOKEN822_DOT)))
	 OUTLEFT
       wordok = 0;
       continue;
     case TOKEN822_ATOM: case TOKEN822_QUOTE: case TOKEN822_LITERAL:
       if (!wordok)
	 FLUSHCOMMA
       wordok = 0;
       ADDRLEFT
       continue;
     case TOKEN822_COMMENT:
       /* comment is lexically a space; shouldn't affect wordok */
       break;
     case TOKEN822_COMMA:
       FLUSH
       wordok = 1;
       break;
     default:
       wordok = 1;
       ADDRLEFT
       continue;
    }
   OUTLEFT
  }
 FLUSH
 ++t;
 while (t > ta->t)
   if (!token822_append(taout,--t)) return -1;

 token822_reverse(taout);
 return 1;
}
