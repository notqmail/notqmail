#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "auto_break.h"
#include "auto_usera.h"
#include "byte.h"
#include "case.h"
#include "cdb.h"
#include "constmap.h"
#include "error.h"
#include "fmt.h"
#include "open.h"
#include "str.h"
#include "stralloc.h"
#include "uint32.h"
#include "substdio.h"
#include "env.h"

extern void die_nomem();
extern void die_control();
extern void die_cdb();
extern void die_sys();

static stralloc envnoathost = {0};
static stralloc percenthack = {0};
static stralloc locals = {0};
static stralloc vdoms = {0};
static struct constmap mappercenthack;
static struct constmap maplocals;
static struct constmap mapvdoms;

static char *dash;
static char *extension;
static char *local;
static struct passwd *pw;

static char errbuf[128];
static struct substdio sserr = SUBSTDIO_FDBUF(write,2,errbuf,sizeof errbuf);

static char pidbuf[64];
static char remoteipbuf[64]=" ";

static int flagdenyall;
static int flagdenyany;

void realrcptto_init()
{
  char *x;

  if (control_rldef(&envnoathost,"control/envnoathost",1,"envnoathost") != 1)
    die_control();

  if (control_readfile(&locals,"control/locals",1) != 1) die_control();
  if (!constmap_init(&maplocals,locals.s,locals.len,0)) die_nomem();
  switch(control_readfile(&percenthack,"control/percenthack",0)) {
    case -1: die_control();
    case 0: if (!constmap_init(&mappercenthack,"",0,0)) die_nomem();
    case 1:
      if (!constmap_init(&mappercenthack,percenthack.s,percenthack.len,0))
        die_nomem();
  }
  switch(control_readfile(&vdoms,"control/virtualdomains",0)) {
    case -1: die_control();
    case 0: if (!constmap_init(&mapvdoms,"",0,1)) die_nomem();
    case 1: if (!constmap_init(&mapvdoms,vdoms.s,vdoms.len,1)) die_nomem();
  }

  str_copy(pidbuf + fmt_ulong(pidbuf,getpid())," ");
  x=env_get("PROTO");
  if (x) {
    static char const remoteip[]="REMOTEIP";
    unsigned int len = str_len(x);
    if (len <= sizeof remoteipbuf - sizeof remoteip) {
      byte_copy(remoteipbuf,len,x);
      byte_copy(remoteipbuf + len,sizeof remoteip,remoteip);
      x = env_get(remoteipbuf);
      len = str_len(x);
      if (len + 1 < sizeof remoteipbuf) {
        byte_copy(remoteipbuf,len,x);
        remoteipbuf[len]=' ';
        remoteipbuf[len + 1]='\0';
      }
    }
  }

  x = env_get("QMAILRRTDENYALL");
  flagdenyall = (x && x[0]=='1' && x[1]=='\0');
}

void realrcptto_start()
{
  flagdenyany = 0;
}

static int denyaddr(addr)
char *addr;
{
  substdio_puts(&sserr,"realrcptto ");
  substdio_puts(&sserr,pidbuf);
  substdio_puts(&sserr,remoteipbuf);
  substdio_puts(&sserr,addr);
  substdio_puts(&sserr,"\n");
  substdio_flush(&sserr);
  flagdenyany = 1;
  return flagdenyall;
}

static void stat_error(path,error)
char* path;
int error;
{
  substdio_puts(&sserr,"unable to stat ");
  substdio_puts(&sserr,path);
  substdio_puts(&sserr,": ");
  substdio_puts(&sserr,error_str(error));
  substdio_puts(&sserr,"\n");
  substdio_flush(&sserr);
}

#define GETPW_USERLEN 32

static int userext()
{
  char username[GETPW_USERLEN];
  struct stat st;

  extension = local + str_len(local);
  for (;;) {
    if (extension - local < sizeof(username))
      if (!*extension || (*extension == *auto_break)) {
	byte_copy(username,extension - local,local);
	username[extension - local] = 0;
	case_lowers(username);
	errno = 0;
	pw = getpwnam(username);
	if (errno == error_txtbsy) die_sys();
	if (pw)
	  if (pw->pw_uid)
	    if (stat(pw->pw_dir,&st) == 0) {
	      if (st.st_uid == pw->pw_uid) {
		dash = "";
		if (*extension) { ++extension; dash = "-"; }
		return 1;
	      }
	    }
	    else
	      if (error_temp(errno)) die_sys();
      }
    if (extension == local) return 0;
    --extension;
  }
}

int realrcptto(addr)
char *addr;
{
  char *homedir;
  static stralloc localpart = {0};
  static stralloc lower = {0};
  static stralloc nughde = {0};
  static stralloc wildchars = {0};
  static stralloc safeext = {0};
  static stralloc qme = {0};
  unsigned int i,at;

  /* Short circuit, or full logging?  Short circuit. */
  if (flagdenyall && flagdenyany) return 1;

  /* qmail-send:rewrite */
  if (!stralloc_copys(&localpart,addr)) die_nomem();
  i = byte_rchr(localpart.s,localpart.len,'@');
  if (i == localpart.len) {
    if (!stralloc_cats(&localpart,"@")) die_nomem();
    if (!stralloc_cat(&localpart,&envnoathost)) die_nomem();
  }
  while (constmap(&mappercenthack,localpart.s + i + 1,localpart.len - i - 1)) {
    unsigned int j = byte_rchr(localpart.s,i,'%');
    if (j == i) break;
    localpart.len = i;
    i = j;
    localpart.s[i] = '@';
  }
  at = byte_rchr(localpart.s,localpart.len,'@');
  if (constmap(&maplocals,localpart.s + at + 1,localpart.len - at - 1)) {
    localpart.len = at;
    localpart.s[at] = '\0';
  } else {
    unsigned int xlen,newlen;
    char *x;
    for (i = 0;;++i) {
      if (i > localpart.len) return 1;
      if (!i || (i == at + 1) || (i == localpart.len) ||
          ((i > at) && (localpart.s[i] == '.'))) {
        x = constmap(&mapvdoms,localpart.s + i,localpart.len - i);
        if (x) break;
      }
    }
    if (!*x) return 1;
    xlen = str_len(x) + 1;  /* +1 for '-' */
    newlen = xlen + at + 1; /* +1 for \0 */
    if (xlen < 1 || newlen - 1 < xlen || newlen < 1 ||
        !stralloc_ready(&localpart,newlen))
      die_nomem();
    localpart.s[newlen - 1] = '\0';
    byte_copyr(localpart.s + xlen,at,localpart.s);
    localpart.s[xlen - 1] = '-';
    byte_copy(localpart.s,xlen - 1,x);
    localpart.len = newlen;
  }

  /* qmail-lspawn:nughde_get */
  {
    int r,fd,flagwild;
    if (!stralloc_copys(&lower,"!")) die_nomem();
    if (!stralloc_cats(&lower,localpart.s)) die_nomem();
    if (!stralloc_0(&lower)) die_nomem();
    case_lowerb(lower.s,lower.len);
    if (!stralloc_copys(&nughde,"")) die_nomem();
    fd = open_read("users/cdb");
    if (fd == -1) {
      if (errno != error_noent) die_cdb();
    } else {
      uint32 dlen;
      r = cdb_seek(fd,"",0,&dlen);
      if (r != 1) die_cdb();
      if (!stralloc_ready(&wildchars,(unsigned int) dlen)) die_nomem();
      wildchars.len = dlen;
      if (cdb_bread(fd,wildchars.s,wildchars.len) == -1) die_cdb();
      i = lower.len;
      flagwild = 0;
      do { /* i > 0 */
        if (!flagwild || (i == 1) ||
            (byte_chr(wildchars.s,wildchars.len,lower.s[i - 1])
             < wildchars.len)) {
          r = cdb_seek(fd,lower.s,i,&dlen);
          if (r == -1) die_cdb();
          if (r == 1) {
            char *x;
            if (!stralloc_ready(&nughde,(unsigned int) dlen)) die_nomem();
            nughde.len = dlen;
            if (cdb_bread(fd,nughde.s,nughde.len) == -1) die_cdb();
            if (flagwild)
              if (!stralloc_cats(&nughde,localpart.s + i - 1)) die_nomem();
            if (!stralloc_0(&nughde)) die_nomem();
            close(fd);
            x=nughde.s;
            /* skip username */
            x += byte_chr(x,nughde.s + nughde.len - x,'\0');
            if (x == nughde.s + nughde.len) return 1;
            ++x;
            /* skip uid */
            x += byte_chr(x,nughde.s + nughde.len - x,'\0');
            if (x == nughde.s + nughde.len) return 1;
            ++x;
            /* skip gid */
            x += byte_chr(x,nughde.s + nughde.len - x,'\0');
            if (x == nughde.s + nughde.len) return 1;
            ++x;
            /* skip homedir */
            homedir=x;
            x += byte_chr(x,nughde.s + nughde.len - x,'\0');
            if (x == nughde.s + nughde.len) return 1;
            ++x;
            /* skip dash */
            dash=x;
            x += byte_chr(x,nughde.s + nughde.len - x,'\0');
            if (x == nughde.s + nughde.len) return 1;
            ++x;
            extension=x;
            goto got_nughde;
          }
        }
        --i;
        flagwild = 1;
      } while (i);
      close(fd);
    }
  }

  /* qmail-getpw */
  local = localpart.s;
  if (!userext()) {
    extension = local;
    dash = "-";
    pw = getpwnam(auto_usera);
  }
  if (!pw) return denyaddr(addr);
  if (!stralloc_copys(&nughde,pw->pw_dir)) die_nomem();
  if (!stralloc_0(&nughde)) die_nomem();
  homedir=nughde.s;

  got_nughde:

  /* qmail-local:qmesearch */
  if (!*dash) return 1;
  if (!stralloc_copys(&safeext,extension)) die_nomem();
  case_lowerb(safeext.s,safeext.len);
  for (i = 0;i < safeext.len;++i)
    if (safeext.s[i] == '.')
      safeext.s[i] = ':';
  {
    struct stat st;
    int i;
    if (!stralloc_copys(&qme,homedir)) die_nomem();
    if (!stralloc_cats(&qme,"/.qmail")) die_nomem();
    if (!stralloc_cats(&qme,dash)) die_nomem();
    if (!stralloc_cat(&qme,&safeext)) die_nomem();
    if (!stralloc_0(&qme)) die_nomem();
    if (stat(qme.s,&st) == 0) return 1;
    if (errno != error_noent) {
      stat_error(qme.s,errno);
      return 1;
    }
    for (i = safeext.len;i >= 0;--i)
      if (!i || (safeext.s[i - 1] == '-')) {
        if (!stralloc_copys(&qme,homedir)) die_nomem();
        if (!stralloc_cats(&qme,"/.qmail")) die_nomem();
        if (!stralloc_cats(&qme,dash)) die_nomem();
        if (!stralloc_catb(&qme,safeext.s,i)) die_nomem();
        if (!stralloc_cats(&qme,"default")) die_nomem();
        if (!stralloc_0(&qme)) die_nomem();
        if (stat(qme.s,&st) == 0) return 1;
        if (errno != error_noent) {
          stat_error(qme.s,errno);
          return 1;
        }
      }
    return denyaddr(addr);
  }
}

int realrcptto_deny()
{
  return flagdenyall && flagdenyany;
}
