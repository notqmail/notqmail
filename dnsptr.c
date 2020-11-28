#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "str.h"
#include "scan.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"

stralloc sa = {0};
struct ip_address ip;

int main(int argc, char **argv)
{
 if (argc == 1) return 100;

 ip_scan(argv[1],&ip);

 dns_init(0);
 dnsdoe(dns_ptr(&sa,&ip));
 substdio_putflush(subfdout,sa.s,sa.len);
 substdio_putsflush(subfdout,"\n");
 return 0;
}
