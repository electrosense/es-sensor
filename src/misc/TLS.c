/*
 * Copyright (C) 2015 by Damian Pfammatter <pfammatterdamian@gmail.com>
 *
 * This file is part of RTL-spec.
 *
 * RTL-Spec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * RTL-Spec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RTL-Spec.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h>

#include "TLS.h"

static void tls_info(TLS_Connection *c, const int err, const char *msg) {
  int portnum;
  char hostaddr[INET_ADDRSTRLEN];
  
  if(c != NULL) {
    portnum = ntohs(c->tcp_c->host_addr.sin_port);
    inet_ntop(AF_INET, &(c->tcp_c->host_addr.sin_addr), hostaddr, INET_ADDRSTRLEN);
  } else {
    portnum = -1;
    strncpy(hostaddr, "empty", INET_ADDRSTRLEN);
  }
  fprintf(stderr, "%s\tTLS %s:%d\t%s:%d\n", err ? "[ERROR]" : "[OK]", msg, err, hostaddr, portnum);
  fprintf(stderr, "\t\t%s\n", err ? strerror(errno) : "");
}

int tls_init(TLS_Connection **tls_c, TCP_Connection *tcp_c, const SSL_METHOD *method,
	     const char *ca_cert_file, const char *cert_file, const char *key_file,
	     const char *hostaddr, const int portnum) {
  int server_mode;
  int verify_flags;
  
  server_mode = (tcp_c == NULL ? 0 : 1);
  *tls_c = (TLS_Connection *) malloc(sizeof(TLS_Connection));
  
  // Initialize SSL/TLS Library
  SSL_load_error_strings();
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  // Initialize TCP Connection
  if(server_mode) {
    (*tls_c)->tcp_c = tcp_c;
  } else {
    if(tcp_init(&((*tls_c)->tcp_c), hostaddr, portnum) < 0) return -1;
  }
  // Initialize SSL/TLS context
  (*tls_c)->ssl_ctx = SSL_CTX_new(method);
  if((*tls_c)->ssl_ctx == NULL) return -2;
  // Require peer authentication
  verify_flags = SSL_VERIFY_PEER;
  if(server_mode) verify_flags |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
  SSL_CTX_set_verify((*tls_c)->ssl_ctx, verify_flags, NULL);
  // Peer certificate must be directly signed by the CA certificate
  SSL_CTX_set_verify_depth((*tls_c)->ssl_ctx, 1);
  // Load certificate and private key
  if(SSL_CTX_use_certificate_file((*tls_c)->ssl_ctx, cert_file, SSL_FILETYPE_PEM) <= 0) return -3;
  if(SSL_CTX_use_PrivateKey_file((*tls_c)->ssl_ctx, key_file, SSL_FILETYPE_PEM) <= 0) return -4;
  if(SSL_CTX_check_private_key((*tls_c)->ssl_ctx) <= 0) return -5;
  // Load trusted CA certificate
  if(SSL_CTX_load_verify_locations((*tls_c)->ssl_ctx, ca_cert_file, NULL) <= 0) return -6;
  
  return 0;
}

int tls_init_p(TLS_Connection **tls_c, TCP_Connection *tcp_c, const SSL_METHOD *method,
	       const char *ca_cert_file, const char *cert_file, const char *key_file,
	       const char *hostaddr, const int portnum) {
  int err = tls_init(tls_c, tcp_c, method, ca_cert_file, cert_file, key_file, hostaddr, portnum);
  tls_info(*tls_c, err, "init");
  return err;
}

int tls_accept(TLS_Connection *c) {
  // SSL/TLS connection state
  c->ssl = SSL_new(c->ssl_ctx);
  if(c->ssl == NULL) return -1;
  // Attach TCP to SSL/TLS connection
  if(SSL_set_fd(c->ssl, c->tcp_c->socket_fd) <= 0) return -2;
  // Accept SSL/TLS connection
  if(SSL_accept(c->ssl) <= 0) return -3;
  return 0;
}

int tls_accept_p(TLS_Connection *c) {
  int err = tls_accept(c);
  tls_info(c, err, "accept");
  return err;
}

int tls_connect(TLS_Connection *c) {
  // Open TCP Connection
  if(tcp_connect(c->tcp_c) < 0) return -1;
  // SSL/TLS connection state
  c->ssl = SSL_new(c->ssl_ctx);
  if(c->ssl == NULL) return -2;
  // Attach TCP to SSL/TLS connection
  if(SSL_set_fd(c->ssl, c->tcp_c->socket_fd) <= 0) return -3;
  // Open SSL/TLS connection
  if(SSL_connect(c->ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    return -4;
  }
  return 0;
}

int tls_connect_p(TLS_Connection *c) {
  int err = tls_connect(c);
  tls_info(c, err, "connect");
  return err;
}

// int tls_authenticate(TLS_Connection *c, const char *ca_cert_file) {
//   
//   X509 *peer_cert = NULL;
//   X509_LOOKUP *lookup = NULL;
//   X509_STORE *store = NULL;
//   X509_STORE_CTX *store_ctx = NULL;
// 
//   // X509 Peer Certificate
//   peer_cert = SSL_get_peer_certificate(c->ssl);
//   if(peer_cert == NULL) return -1;
// #if defined(VERBOSE)
//   // Show Certificate
//   char *line;
//   fprintf(stderr, "=== PEER CERT ===\n");
//   line = X509_NAME_oneline(X509_get_subject_name(peer_cert), 0, 0);
//   fprintf(stderr, "Subject: %s\n", line);
//   free(line);
//   line = X509_NAME_oneline(X509_get_issuer_name(peer_cert), 0, 0);
//   fprintf(stderr, "Issuer : %s\n", line);
//   free(line);
//   fprintf(stderr, "=================\n");
// #endif
//   // X509 Certificate Store
//   store = X509_STORE_new();
//   X509_STORE_set_flags(store, 0);
//   // X509 Certificate Lookup
//   lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
//   if(!X509_LOOKUP_load_file(lookup, ca_cert_file, X509_FILETYPE_PEM)) return -2;
//   lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
//   if(!X509_LOOKUP_add_dir(lookup, NULL, X509_FILETYPE_DEFAULT)) return -3;
//   // X509 Certificate Store Context
//   store_ctx = X509_STORE_CTX_new();
//   if(!X509_STORE_CTX_init(store_ctx, store, peer_cert, 0)) return -4;
//   if(X509_verify_cert(store_ctx) <= 0) return -5;
// 
//   if(peer_cert != NULL) X509_free(peer_cert);
//   if(store != NULL) X509_STORE_free(store);
//   if(store_ctx != NULL) X509_STORE_CTX_free(store_ctx);
//   
//   return 0;
// }

// int tls_authenticate_p(TLS_Connection *c, const char *ca_cert_file) {
//   int err = tls_authenticate(c, ca_cert_file);
//   tls_info(c, err, "authenticate");
//   return err;
// }

int tls_read(TLS_Connection *c, void *msg, size_t size) {
  int bytes_read = 0, cnt;
  while(bytes_read < size) {
    cnt = SSL_read(c->ssl, msg+bytes_read, size-bytes_read);
    if(cnt < 0) return -1;
    if(cnt == 0) break;
    bytes_read += cnt;
  }
  return 0;
}

int tls_read_p(TLS_Connection *c, void *msg, size_t size) {
  int err = tls_read(c, msg, size);
  tls_info(c, err, "read");
  return err;
}

int tls_write(TLS_Connection *c, const void *msg, const int len) {
  int cnt = SSL_write(c->ssl, msg, len);
  return cnt==len ? 0 : -1;
}

int tls_write_p(TLS_Connection *c, const void *msg, const int len) {
  int err = tls_write(c, msg, len);
  tls_info(c, err, "write");
  return err;
}

int tls_disconnect(TLS_Connection *c) {
  int err, i;
  if(c == NULL) return 0;
  // Close SSL Connection
  for(i=0; i<5; i++) {
    err = SSL_shutdown(c->ssl);
    if(err == 1) break;
  }
  if(err != 1) return -1;
  // Close TCP Connection
  if(tcp_disconnect(c->tcp_c) < 0) return -2;
  return 0;
}

int tls_disconnect_p(TLS_Connection *c) {
  int err = tls_disconnect(c);
  tls_info(c, err, "disconnect");
  return err;
}

int tls_release(TLS_Connection *c) {
  // Release SSL/TLS Connection
  if(c->ssl != NULL) SSL_free(c->ssl);
  // Release SSL/TLS Context
  if(c->ssl_ctx != NULL) SSL_CTX_free(c->ssl_ctx);
  // Release TCP Connection
  if(tcp_release(c->tcp_c) < 0) return -1;
  // Release SSL/TLS Library
  EVP_cleanup();
  ERR_free_strings();
  free(c);
  c = NULL;
  return 0;
}

int tls_release_p(TLS_Connection *c) {
  int portnum, err;
  char hostaddr[INET_ADDRSTRLEN];
  portnum = ntohs(c->tcp_c->host_addr.sin_port);
  inet_ntop(AF_INET, &(c->tcp_c->host_addr.sin_addr), hostaddr, INET_ADDRSTRLEN);
  err = tls_release(c);
  fprintf(stderr, "%s\tTLS %s:%d\t%s:%d\n", err ? "[ERROR]" : "[OK]", "release", err, hostaddr, portnum);
  fprintf(stderr, "\t%s\n", err ? strerror(errno) : "");
  return err;
}
