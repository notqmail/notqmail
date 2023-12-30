#ifndef TLS_H
#define TLS_H

#include <openssl/ssl.h>

extern int smtps;
extern SSL *ssl;

void ssl_free(SSL *myssl);
void ssl_exit(int status);
# define _exit ssl_exit

const char *ssl_error();
const char *ssl_error_str();

#endif
