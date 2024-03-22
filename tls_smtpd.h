#ifndef TLS_SMTPD_H
#define TLS_SMTPD_H

#define TLS

#ifdef TLS

#include "ssl_timeoutio.h"
#include "tls.h"

extern int ssl_rfd;
extern int ssl_wfd;

void tls_init(void);
int tls_verify(void);
void tls_nogateway(void);
int tls_cert_available(void);
void smtp_tls(const char *arg);
void flush_io(void);

#else /* TLS */

static inline int tls_verify(void)
{
  return 0;
}

static inline void tls_nogateway(void)
{
}

static inline int tls_cert_available()
{
  return 0;
}

#define smtp_tls err_unimpl
#define flush_io flush

#endif /* TLS */

#endif
