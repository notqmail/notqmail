#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include "sig.h"
#include "stralloc.h"
#include "str.h"
#include "env.h"
#include "fmt.h"
#include "scan.h"
#include "subgetopt.h"
#include "ip.h"
#include "dns.h"
#include "byte.h"
#include "remoteinfo.h"
#include "exit.h"
#include "case.h"

void die() { _exit(111); }

struct sockaddr_in salocal;
unsigned long localport;
struct ip_address iplocal;
stralloc localname = {0};

struct sockaddr_in saremote;
unsigned long remoteport;
struct ip_address ipremote;
stralloc remotename = {0};

char temp[IPFMT + FMT_ULONG];

void main(argc,argv)
int argc;
char *argv[];
{
 int dummy;
 char *proto;
 int opt;
 int flagremoteinfo;
 unsigned long timeout;

 sig_pipeignore();

 flagremoteinfo = 1;
 timeout = 30;
 while ((opt = sgopt(argc,argv,"rRt:")) != sgoptdone)
   switch(opt)
    {
     case 'r': flagremoteinfo = 1; break;
     case 'R': flagremoteinfo = 0; break;
     case 't': scan_ulong(sgoptarg,&timeout); break;
    }

 argv += sgoptind;
 argc -= sgoptind;

 if (argc < 1) die();
 if (!env_init()) die();

 proto = env_get("PROTO");
 if (!proto || str_diff(proto,"TCP"))
  {
   if (!env_put("PROTO=TCP")) die();

   dummy = sizeof(salocal);
   if (getsockname(0,(struct sockaddr *) &salocal,&dummy) == -1) die();

   localport = ntohs(salocal.sin_port);
   temp[fmt_ulong(temp,localport)] = 0;
   if (!env_put2("TCPLOCALPORT",temp)) die();

   byte_copy(&iplocal,4,&salocal.sin_addr);
   temp[ip_fmt(temp,&iplocal)] = 0;
   if (!env_put2("TCPLOCALIP",temp)) die();

   switch(dns_ptr(&localname,&iplocal))
    {
     case DNS_MEM: die();
     case DNS_SOFT:
       if (!stralloc_copys(&localname,"softdnserror")) die();
     case 0:
       if (!stralloc_0(&localname)) die();
       case_lowers(localname.s);
       if (!env_put2("TCPLOCALHOST",localname.s)) die();
       break;
     default:
       if (!env_unset("TCPLOCALHOST")) die();
    }

   dummy = sizeof(saremote);
   if (getpeername(0,(struct sockaddr *) &saremote,&dummy) == -1) die();

   remoteport = ntohs(saremote.sin_port);
   temp[fmt_ulong(temp,remoteport)] = 0;
   if (!env_put2("TCPREMOTEPORT",temp)) die();

   byte_copy(&ipremote,4,&saremote.sin_addr);
   temp[ip_fmt(temp,&ipremote)] = 0;
   if (!env_put2("TCPREMOTEIP",temp)) die();

   switch(dns_ptr(&remotename,&ipremote))
    {
     case DNS_MEM: die();
     case DNS_SOFT:
       if (!stralloc_copys(&remotename,"softdnserror")) die();
     case 0:
       if (!stralloc_0(&remotename)) die();
       case_lowers(remotename.s);
       if (!env_put2("TCPREMOTEHOST",remotename.s)) die();
       break;
     default:
       if (!env_unset("TCPREMOTEHOST")) die();
    }

   if (!env_unset("TCPREMOTEINFO")) die();
   if (flagremoteinfo)
    {
     char *rinfo;
     rinfo = remoteinfo_get(&ipremote,remoteport,&iplocal,localport,(int) timeout);
     if (rinfo)
       if (!env_put2("TCPREMOTEINFO",rinfo)) die();
    }
  }

 sig_pipedefault();
 execvp(*argv,argv);
 die();
}
