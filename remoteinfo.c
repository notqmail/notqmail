#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "byte.h"
#include "substdio.h"
#include "ip.h"
#include "fmt.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "remoteinfo.h"

static char line[999];
static int t;

static int mywrite(fd,buf,len) int fd; char *buf; int len;
{
  return timeoutwrite(t,fd,buf,len);
}
static int myread(fd,buf,len) int fd; char *buf; int len;
{
  return timeoutread(t,fd,buf,len);
}

char *remoteinfo_get(ipr,rp,ipl,lp,timeout)
struct ip_address *ipr;
unsigned long rp;
struct ip_address *ipl;
unsigned long lp;
int timeout;
{
  char *x;
  int s;
  struct sockaddr_in sin;
  substdio ss;
  char buf[32];
  unsigned int len;
  int numcolons;
  char ch;

  t = timeout;
 
  s = socket(AF_INET,SOCK_STREAM,0);
  if (s == -1) return 0;
 
  byte_zero(&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  byte_copy(&sin.sin_addr,4,ipl);
  sin.sin_port = 0;
  if (bind(s,(struct sockaddr *) &sin,sizeof(sin)) == -1) { close(s); return 0; }
  if (timeoutconn(s,ipr,113,timeout) == -1) { close(s); return 0; }
  fcntl(s,F_SETFL,fcntl(s,F_GETFL,0) & ~O_NDELAY);
 
  len = 0;
  len += fmt_ulong(line + len,rp);
  len += fmt_str(line + len," , ");
  len += fmt_ulong(line + len,lp);
  len += fmt_str(line + len,"\r\n");
 
  substdio_fdbuf(&ss,mywrite,s,buf,sizeof buf);
  if (substdio_putflush(&ss,line,len) == -1) { close(s); return 0; }
 
  substdio_fdbuf(&ss,myread,s,buf,sizeof buf);
  x = line;
  numcolons = 0;
  for (;;) {
    if (substdio_get(&ss,&ch,1) != 1) { close(s); return 0; }
    if ((ch == ' ') || (ch == '\t') || (ch == '\r')) continue;
    if (ch == '\n') break;
    if (numcolons < 3) { if (ch == ':') ++numcolons; }
    else { *x++ = ch; if (x == line + sizeof(line) - 1) break; }
  }
  *x = 0;
  close(s);
  return line;
}
