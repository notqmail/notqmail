#ifndef IP_H
#define IP_H

struct ip_address { unsigned char d[4]; } ;
typedef struct  ip_address ip_addr;

extern unsigned int ip_fmt();
#define IPFMT 19
extern unsigned int ip_scan();
extern unsigned int ip_scanbracket();

#endif
