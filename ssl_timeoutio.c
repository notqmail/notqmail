#include "select.h"
#include "error.h"
#include "ndelay.h"
#include "now.h"
#include "ssl_timeoutio.h"

int ssl_timeoutio(int (*fun)(),
  int t, int rfd, int wfd, SSL *ssl, char *buf, int len)
{
  int n;
  const datetime_sec end = (datetime_sec)t + now();

  do {
    fd_set fds;
    struct timeval tv;

    const int r = buf ? fun(ssl, buf, len) : fun(ssl);
    if (r > 0) return r;

    t = end - now();
    if (t < 0) break;
    tv.tv_sec = (time_t)t; tv.tv_usec = 0;

    FD_ZERO(&fds);
    switch (SSL_get_error(ssl, r))
    {
    default: return r; /* some other error */
    case SSL_ERROR_WANT_READ:
      FD_SET(rfd, &fds); n = select(rfd + 1, &fds, NULL, NULL, &tv);
      break;
    case SSL_ERROR_WANT_WRITE:
      FD_SET(wfd, &fds); n = select(wfd + 1, NULL, &fds, NULL, &tv);
      break;
    }

    /* n is the number of descriptors that changed status */
  } while (n > 0);

  if (n != -1) errno = error_timeout;
  return -1;
}

int ssl_timeoutaccept(int t, int rfd, int wfd, SSL *ssl)
{
  int r;

  /* if connection is established, keep NDELAY */
  if (ndelay_on(rfd) == -1 || ndelay_on(wfd) == -1) return -1;
  r = ssl_timeoutio(SSL_accept, t, rfd, wfd, ssl, NULL, 0);

  if (r <= 0) { ndelay_off(rfd); ndelay_off(wfd); }
  else SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

  return r;
}

int ssl_timeoutconn(int t, int rfd, int wfd, SSL *ssl)
{
  int r;

  /* if connection is established, keep NDELAY */
  if (ndelay_on(rfd) == -1 || ndelay_on(wfd) == -1) return -1;
  r = ssl_timeoutio(SSL_connect, t, rfd, wfd, ssl, NULL, 0);

  if (r <= 0) { ndelay_off(rfd); ndelay_off(wfd); }
  else SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

  return r;
}

int ssl_timeoutrehandshake(int t, int rfd, int wfd, SSL *ssl)
{
  int r;

  SSL_renegotiate(ssl);
  r = ssl_timeoutio(SSL_do_handshake, t, rfd, wfd, ssl, NULL, 0);
  if (r <= 0 || ssl->type == SSL_ST_CONNECT) return r;

  /* this is for the server only */
  ssl->state = SSL_ST_ACCEPT;
  return ssl_timeoutio(SSL_do_handshake, t, rfd, wfd, ssl, NULL, 0);
}

int ssl_timeoutread(int t, int rfd, int wfd, SSL *ssl, char *buf, int len)
{
  if (!buf) return 0;
  if (SSL_pending(ssl)) return SSL_read(ssl, buf, len);
  return ssl_timeoutio(SSL_read, t, rfd, wfd, ssl, buf, len);
}

int ssl_timeoutwrite(int t, int rfd, int wfd, SSL *ssl, char *buf, int len)
{
  if (!buf) return 0;
  return ssl_timeoutio(SSL_write, t, rfd, wfd, ssl, buf, len);
}
