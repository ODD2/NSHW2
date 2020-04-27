/*
 * ssl_helper.h
 *
 *  Created on: Apr 27, 2020
 *      Author: xd
 */

#ifndef SSL_HELPER_H_
#define SSL_HELPER_H_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void init_openssl();

void cleanup_openssl();

SSL_CTX* create_context(bool server);

void configure_context(SSL_CTX*, const char[], const char[]);

#endif /* SSL_HELPER_H_ */
