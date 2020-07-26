#include "ssl-smtpd.h"

#include "alloc.h"
#include "constmap.h"
#include "control.h"
#include "ssl_timeoutio.h"
#include "stralloc.h"
#include "substdio.h"
#include "tls.h"

/* implemented in qmail-smtpd.c */
extern void dohelo(char *arg);
extern void err_unimpl(char *arg);
extern void flush();
extern void out(char *s);

extern const char *protocol;
extern char *relayclient;
extern char *remotehost;
extern substdio ssin;
extern substdio ssout;
extern int timeout;

stralloc proto = {0};
int ssl_verified = 0;
const char *ssl_verify_err = 0;
int ssl_rfd = -1;
int ssl_wfd = -1;

void smtp_tls(char *arg)
{
  if (ssl) err_unimpl(NULL);
  else if (*arg) out("501 Syntax error (no parameters allowed) (#5.5.4)\r\n");
  else tls_init();
}

RSA *tmp_rsa_cb(SSL *ssl, int export, int keylen)
{
  RSA *rsa;

  if (!export) keylen = 2048;
  if (keylen == 2048) {
    FILE *in = fopen("control/rsa2048.pem", "r");
    if (in) {
      rsa = PEM_read_RSAPrivateKey(in, NULL, NULL, NULL);
      fclose(in);
      if (rsa) return rsa;
    }
  }
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  BIGNUM *e; /*exponent */
  e = BN_new();
  BN_set_word(e, RSA_F4);
  if (RSA_generate_key_ex(rsa, keylen, e, NULL) == 1)
    return rsa;
  return NULL;
#else
  return RSA_generate_key(keylen, RSA_F4, NULL, NULL);
#endif
}

DH *tmp_dh_cb(SSL *ssl, int export, int keylen)
{
  DH *dh;

  if (!export) keylen = 2048;
  if (keylen == 2048) {
    FILE *in = fopen("control/dh2048.pem", "r");
    if (in) {
      dh = PEM_read_DHparams(in, NULL, NULL, NULL);
      fclose(in);
      if (dh) return dh;
    }
  }
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  if((dh = DH_new()) && (DH_generate_parameters_ex(dh, keylen, DH_GENERATOR_2, NULL) == 1))
    return dh;
  return NULL;
#else
  return DH_generate_parameters(keylen, DH_GENERATOR_2, NULL, NULL);
#endif
}

/* don't want to fail handshake if cert isn't verifiable */
int verify_cb(int preverify_ok, X509_STORE_CTX *x509_ctx) { return 1; }

void tls_nogateway()
{
  /* there may be cases when relayclient is set */
  if (!ssl || relayclient) return;
  out("; no valid cert for gatewaying");
  if (ssl_verify_err) { out(": "); out(ssl_verify_err); }
}
void tls_out(const char *s1, const char *s2)
{
  out("454 TLS "); out(s1);
  if (s2) { out(": "); out(s2); }
  out(" (#4.3.0)\r\n"); flush();
}
void tls_err(const char *s) { tls_out(s, ssl_error()); if (smtps) die_read(); }

# define CLIENTCA "control/clientca.pem"
# define CLIENTCRL "control/clientcrl.pem"
# define SERVERCERT "control/servercert.pem"

int tls_verify()
{
  stralloc clients = {0};
  struct constmap mapclients;

  if (!ssl || relayclient || ssl_verified) return 0;
  ssl_verified = 1; /* don't do this twice */

  /* request client cert to see if it can be verified by one of our CAs
   * and the associated email address matches an entry in tlsclients */
  switch (control_readfile(&clients, "control/tlsclients", 0))
  {
  case 1:
    if (constmap_init(&mapclients, clients.s, clients.len, 0)) {
      /* if CLIENTCA contains all the standard root certificates, a
       * 0.9.6b client might fail with SSL_R_EXCESSIVE_MESSAGE_SIZE;
       * it is probably due to 0.9.6b supporting only 8k key exchange
       * data while the 0.9.6c release increases that limit to 100k */
      STACK_OF(X509_NAME) *sk = SSL_load_client_CA_file(CLIENTCA);
      if (sk) {
        SSL_set_client_CA_list(ssl, sk);
        SSL_set_verify(ssl, SSL_VERIFY_PEER, verify_cb);
        break;
      }
      constmap_free(&mapclients);
    }
  case 0: alloc_free(clients.s); return 0;
  case -1: die_control();
  }

  if (ssl_timeoutrehandshake(timeout, ssl_rfd, ssl_wfd, ssl) <= 0) {
    const char *err = ssl_error_str();
    tls_out("rehandshake failed", err); die_read();
  }

  do { /* one iteration */
    X509 *peercert;
    X509_NAME *subj;
    stralloc email = {0};

    int n = SSL_get_verify_result(ssl);
    if (n != X509_V_OK)
      { ssl_verify_err = X509_verify_cert_error_string(n); break; }
    peercert = SSL_get_peer_certificate(ssl);
    if (!peercert) break;

    subj = X509_get_subject_name(peercert);
    n = X509_NAME_get_index_by_NID(subj, NID_pkcs9_emailAddress, -1);
    if (n >= 0) {
      const ASN1_STRING *s = X509_NAME_ENTRY_get_data(X509_NAME_get_entry(subj, n));
      if (s) { email.len = s->length; email.s = s->data; }
    }

    if (email.len <= 0)
      ssl_verify_err = "contains no email address";
    else if (!constmap(&mapclients, email.s, email.len))
      ssl_verify_err = "email address not in my list of tlsclients";
    else {
      /* add the cert email to the proto if it helped allow relaying */
      --proto.len;
      if (!stralloc_cats(&proto, "\n  (cert ") /* continuation line */
        || !stralloc_catb(&proto, email.s, email.len)
        || !stralloc_cats(&proto, ")")
        || !stralloc_0(&proto)) die_nomem();
      protocol = proto.s;
      relayclient = "";
      /* also inform qmail-queue */
      if (!env_put("RELAYCLIENT=")) die_nomem();
    }

    X509_free(peercert);
  } while (0);
  constmap_free(&mapclients); alloc_free(clients.s);

  /* we are not going to need this anymore: free the memory */
  SSL_set_client_CA_list(ssl, NULL);
  SSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);

  return relayclient ? 1 : 0;
}

void tls_init()
{
  SSL *myssl;
  SSL_CTX *ctx;
  const char *ciphers;
  stralloc saciphers = {0};
  X509_STORE *store;
  X509_LOOKUP *lookup;
  int session_id_context = 1; /* anything will do */

  SSL_library_init();

  /* a new SSL context with the bare minimum of options */
  ctx = SSL_CTX_new(SSLv23_server_method());
  if (!ctx) { tls_err("unable to initialize ctx"); return; }

  /* POODLE vulnerability */
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

  /* renegotiation should include certificate request */
  SSL_CTX_set_options(ctx, SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

  /* never bother the application with retries if the transport is blocking */
  SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

  /* relevant in renegotiation */
  SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
  if (!SSL_CTX_set_session_id_context(ctx, (void *)&session_id_context,
                                        sizeof(session_id_context)))
    { SSL_CTX_free(ctx); tls_err("failed to set session_id_context"); return; }

  if (!SSL_CTX_use_certificate_chain_file(ctx, SERVERCERT))
    { SSL_CTX_free(ctx); tls_err("missing certificate"); return; }
  SSL_CTX_load_verify_locations(ctx, CLIENTCA, NULL);

  /* crl checking */
  store = SSL_CTX_get_cert_store(ctx);
  if ((lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file())) &&
      (X509_load_crl_file(lookup, CLIENTCRL, X509_FILETYPE_PEM) == 1))
    X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK |
                                X509_V_FLAG_CRL_CHECK_ALL);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  /* support ECDH */
  SSL_CTX_set_ecdh_auto(ctx,1);
#endif

  SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

  /* a new SSL object, with the rest added to it directly to avoid copying */
  myssl = SSL_new(ctx);
  SSL_CTX_free(ctx);
  if (!myssl) { tls_err("unable to initialize ssl"); return; }

  /* this will also check whether public and private keys match */
  if (!SSL_use_RSAPrivateKey_file(myssl, SERVERCERT, SSL_FILETYPE_PEM))
    { SSL_free(myssl); tls_err("no valid RSA private key"); return; }

  ciphers = env_get("TLSCIPHERS");
  if (!ciphers) {
    if (control_readfile(&saciphers, "control/tlsserverciphers", 0) == -1)
      { SSL_free(myssl); die_control(); }
    if (saciphers.len) { /* convert all '\0's except the last one to ':' */
      int i;
      for (i = 0; i < saciphers.len - 1; ++i)
        if (!saciphers.s[i]) saciphers.s[i] = ':';
      ciphers = saciphers.s;
    }
  }
  if (!ciphers || !*ciphers) ciphers = "DEFAULT";
  SSL_set_cipher_list(myssl, ciphers);
  alloc_free(saciphers.s);

  SSL_set_tmp_rsa_callback(myssl, tmp_rsa_cb);
  SSL_set_tmp_dh_callback(myssl, tmp_dh_cb);
  SSL_set_rfd(myssl, ssl_rfd = substdio_fileno(&ssin));
  SSL_set_wfd(myssl, ssl_wfd = substdio_fileno(&ssout));

  if (!smtps) { out("220 ready for tls\r\n"); flush(); }

  if (ssl_timeoutaccept(timeout, ssl_rfd, ssl_wfd, myssl) <= 0) {
    /* neither cleartext nor any other response here is part of a standard */
    const char *err = ssl_error_str();
    tls_out("connection failed", err); ssl_free(myssl); die_read();
  }
  ssl = myssl;

  /* populate the protocol string, used in Received */
  if (!stralloc_copys(&proto, "ESMTPS (")
    || !stralloc_cats(&proto, SSL_get_cipher(ssl))
    || !stralloc_cats(&proto, " encrypted)")) die_nomem();
  if (!stralloc_0(&proto)) die_nomem();
  protocol = proto.s;

  /* have to discard the pre-STARTTLS HELO/EHLO argument, if any */
  dohelo(remotehost);
}
