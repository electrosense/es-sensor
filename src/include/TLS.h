#ifndef TLS_H /* Tansport Layer Security */
#define TLS_H

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "TCP.h"

typedef struct {
  TCP_Connection *tcp_c;
  SSL_CTX        *ssl_ctx;
  SSL            *ssl;
} TLS_Connection;

/*!
 * Initialization of SSL/TLS connection.
 * 
 * Client Mode:
 * 	tcp_c	== NULL	: Initialize new TCP connection for SSL/TLS.
 * 	method		: E.g. TLSv1_1_client_method()
 * 	hostaddr	: Server host address
 * 	portnum		: Server port number
 * 
 * Server Mode:
 * 	tcp_c	!= NULL	: Reuse existing TCP connection for SSL/TLS.
 * 	method		: E.g. TLSv1_1_server_method()
 * 	hostaddr	: not used 
 * 	portnum		: not used
 * 
 * \param tls_c SSL/TLS connection to be initialized
 * \param tcp_c Underlying TCP connection
 * \param method SSL/TLS client or server method
 * \param ca_cert_file CA certificate file to use
 * \param cert_file Certificate file to use
 * \param key_file Private key to use
 * \param hostaddr Remote host address
 * \param portnum Remote host port
 * 
 * \return
 * 	 0: initialization succeeded
 * 	-1: initializing TCP connection failed
 * 	-2: initializing SSL/TLS context failed
 * 	-3: loading certificate file failed
 * 	-4: loading private key file failed
 * 	-5: mismatching certificate and private key
 * 	-6: loading CA certificate failed
 */
int tls_init(TLS_Connection **tls_c, TCP_Connection *tcp_c, const SSL_METHOD *method,
	     const char *ca_cert_file, const char *cert_file, const char *key_file,
	     const char *hostaddr, const int portnum);
int tls_init_p(TLS_Connection **tls_c, TCP_Connection *tcp_c, const SSL_METHOD *method,
	     const char *ca_cert_file, const char *cert_file, const char *key_file,
	     const char *hostaddr, const int portnum);
int tls_accept(TLS_Connection *c);
int tls_accept_p(TLS_Connection *c);
int tls_connect(TLS_Connection *c);
int tls_connect_p(TLS_Connection *c);
int tls_read(TLS_Connection *c, void *msg, size_t size);
int tls_read_p(TLS_Connection *c, void *msg, size_t size);
int tls_write(TLS_Connection *c, const void *msg, const int len);
int tls_write_p(TLS_Connection *c, const void *msg, const int len);
int tls_disconnect(TLS_Connection *c);
int tls_disconnect_p(TLS_Connection *c);
int tls_release(TLS_Connection *c);
int tls_release_p(TLS_Connection *c);

#endif /* TLS_H */