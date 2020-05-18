#ifndef QMAIL_SPP_H
#define QMAIL_SPP_H

extern stralloc sppheaders;
extern int spp_init();
extern int spp_connect();
extern int spp_helo();
extern void spp_rset();
extern int spp_mail();
extern int spp_rcpt();
extern int spp_data();
extern int spp_auth();
extern void spp_rcpt_accepted();

#endif
