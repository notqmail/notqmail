#include "exit.h"
#include "error.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

int smtps = 0;
SSL *ssl = NULL;

void ssl_free(SSL *myssl) { SSL_shutdown(myssl); SSL_free(myssl); }
void ssl_exit(int status) { if (ssl) ssl_free(ssl); _exit(status); }

const char *ssl_error()
{
  int r = ERR_get_error();
  if (!r) return NULL;
  SSL_load_error_strings();
  return ERR_error_string(r, NULL);
}
const char *ssl_error_str()
{
  const char *err = ssl_error();
  if (err) return err;
  if (!errno) return 0;
  return (errno == error_timeout) ? "timed out" : error_str(errno);
}
