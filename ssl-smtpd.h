#ifndef SSL_SMTPD_H
#define SSL_SMTPD_H

void smtp_tls(char *arg);
void tls_init();
int tls_verify();
void tls_nogateway();
extern int ssl_rfd;
extern int ssl_wfd; /* SSL_get_Xfd() are broken */

void ssl_ehlo(void);

#endif
