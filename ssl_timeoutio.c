#include "select.h"
#include "error.h"
#include "ndelay.h"
#include <openssl/ssl.h>

int ssl_timeoutaccept(t,rfd,wfd,ssl) long t; int rfd; int wfd; SSL *ssl;
{
  int r;
  int n = rfd + 1;
  int maxfd = (rfd > wfd ? rfd : wfd) + 1;

  fd_set rfds, wfds;
  fd_set *pwfds = (fd_set *) 0;
  struct timeval tv;
  long end = t + time((long *) 0);

  /* if connection is established, keep it that way */
  if (ndelay_on(rfd) == -1) return -1;
  if (ndelay_on(wfd) == -1) return -1;

  tv.tv_sec = t;
  tv.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(rfd,&rfds);

  /* number of descriptors that changes status */
  while (0 < (n = select(n,&rfds,pwfds,(fd_set *) 0,&tv)))
  {
    r = SSL_accept(ssl);
    if (r > 0) {
      SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
      return r;
    }

    switch (SSL_get_error(ssl, r))
    {
    case SSL_ERROR_WANT_READ:
      pwfds = (fd_set *) 0;
      n = rfd + 1;
      break;
    case SSL_ERROR_WANT_WRITE:
      pwfds = &wfds;
      FD_ZERO(&wfds);
      FD_SET(wfd,&wfds);
      n = maxfd;
      break;
    default:
      /* some other error */
      ndelay_off(rfd);
      ndelay_off(wfd);
      return -2;
    }

    if ((t = end - time((long *)0)) < 0) break;

    tv.tv_sec = t;
    tv.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(rfd,&rfds);
  }

  ndelay_off(rfd);
  ndelay_off(wfd);
  if (n != -1) errno = error_timeout;
  return -1;
}

int ssl_timeoutconn(t,rfd,wfd,ssl) long t; int rfd; int wfd; SSL *ssl;
{
  int r;
  int n = wfd + 1;
  int maxfd = (rfd > wfd ? rfd : wfd) + 1;

  fd_set rfds, wfds;
  fd_set *prfds = (fd_set *) 0;
  struct timeval tv;
  long end = t + time((long *) 0);

  /* if connection is established, keep it that way */
  if (ndelay_on(rfd) == -1) return -1;
  if (ndelay_on(wfd) == -1) return -1;

  tv.tv_sec = t;
  tv.tv_usec = 0;

  FD_ZERO(&wfds);
  FD_SET(wfd,&wfds);

  /* number of descriptors that changes status */
  while (0 < (n = select(n,prfds,&wfds,(fd_set *) 0,&tv)))
  {
    r = SSL_connect(ssl);
    if (r > 0) {
      SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
      return r;
    }

    switch (SSL_get_error(ssl, r))
    {
    case SSL_ERROR_WANT_READ:
      /* try again as SSL_write() might be re-negotiating */
      prfds = &rfds;
      FD_ZERO(&rfds);
      FD_SET(rfd,&rfds);
      n = maxfd;
      break;
    case SSL_ERROR_WANT_WRITE:
      /* try again as network write operation would block */
      prfds = (fd_set *) 0;
      n = wfd + 1;
      break;
    default:
      /* some other error */
      ndelay_off(rfd);
      ndelay_off(wfd);
      return -2;
    }

    if ((t = end - time((long *)0)) < 0) break;

    tv.tv_sec = t;
    tv.tv_usec = 0;

    FD_ZERO(&wfds);
    FD_SET(wfd,&wfds);
  }

  ndelay_off(rfd);
  ndelay_off(wfd);
  if (n != -1) errno = error_timeout;
  return -1;
}

int ssl_timeoutread(t,rfd,wfd,ssl,buf,len)
long t; int rfd; int wfd; SSL *ssl; char *buf; int len;
{
  int r, n, maxfd;
  fd_set rfds, wfds;
  fd_set *pwfds = (fd_set *) 0;
  struct timeval tv;
  long end;

  if (SSL_pending(ssl))
    return SSL_read(ssl,buf,len);

  n = rfd + 1;
  maxfd = (rfd > wfd ? rfd : wfd) + 1;
  end = t + time((long *)0);

  do {
    tv.tv_sec = t;
    tv.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(rfd,&rfds);

    n = select(n,&rfds,pwfds,(fd_set *) 0,&tv);
    if (n == -1) return -1;
    if (n == 0) break; /* timed out */

    r = SSL_read(ssl,buf,len);
    if (r > 0) return r;

    switch (SSL_get_error(ssl, r))
    {
    case SSL_ERROR_WANT_READ:
      /* try again as an incomplete record has been read */
      pwfds = (fd_set *) 0;
      n = rfd + 1;
      break;
    case SSL_ERROR_WANT_WRITE:
      /* try again as SSL_read() might be re-negotiating */
      pwfds = &wfds;
      FD_ZERO(&wfds);
      FD_SET(wfd,&wfds);
      n = maxfd;
      break;
    default:
      /* some other error */
      return -2;
    }
  } while (0 < (t = end - time((long *)0)));

  errno = error_timeout;
  return -1;
}

int ssl_timeoutwrite(t,rfd,wfd,ssl,buf,len)
long t; int rfd; int wfd; SSL* ssl; char *buf; int len;
{
  int r;
  int n = wfd + 1;
  int maxfd = (rfd > wfd ? rfd : wfd) + 1;

  fd_set rfds, wfds;
  fd_set *prfds = (fd_set *) 0;
  struct timeval tv;
  long end = t + time((long *) 0);

  tv.tv_sec = t;
  tv.tv_usec = 0;

  FD_ZERO(&wfds);
  FD_SET(wfd,&wfds);

  /* number of descriptors that changes status */
  while (0 < (n = select(n,prfds,&wfds,(fd_set *) 0,&tv)))
  {
    r = SSL_write(ssl,buf,len);
    if (r > 0) return r;

    switch (SSL_get_error(ssl, r))
    {
    case SSL_ERROR_WANT_READ:
      /* try again as SSL_write() might be re-negotiating */
      prfds = &rfds;
      FD_ZERO(&rfds);
      FD_SET(rfd,&rfds);
      n = maxfd;
      break;
    case SSL_ERROR_WANT_WRITE:
      /* try again as network write operation would block */
      prfds = (fd_set *) 0;
      n = wfd + 1;
      break;
    default:
      /* some other error */
      return -2;
    }

    if ((t = end - time((long *)0)) < 0) break;

    tv.tv_sec = t;
    tv.tv_usec = 0;

    FD_ZERO(&wfds);
    FD_SET(wfd,&wfds);
  }

  if (n != -1) errno = error_timeout;
  return -1;
}
