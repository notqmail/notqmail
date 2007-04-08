#ifndef SSL_TIMEOUTIO_H
#define SSL_TIMEOUTIO_H

#include <openssl/ssl.h>

/* the version is like this: 0xMNNFFPPS: major minor fix patch status */
#if OPENSSL_VERSION_NUMBER < 0x00906000L
# error "Need OpenSSL version at least 0.9.6"
#endif

int ssl_timeoutconn(int t, int rfd, int wfd, SSL *ssl);
int ssl_timeoutaccept(int t, int rfd, int wfd, SSL *ssl);
int ssl_timeoutrehandshake(int t, int rfd, int wfd, SSL *ssl);

int ssl_timeoutread(int t, int rfd, int wfd, SSL *ssl, char *buf, int len);
int ssl_timeoutwrite(int t, int rfd, int wfd, SSL *ssl, char *buf, int len);

int ssl_timeoutio(
  int (*fun)(), int t, int rfd, int wfd, SSL *ssl, char *buf, int len);

#endif
