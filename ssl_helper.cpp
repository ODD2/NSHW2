/*
 * ssl_helper.cpp
 *
 *  Created on: Apr 27, 2020
 *      Author: xd
 */

#include "ssl_helper.h"
#include "GLOBAL.h"

void init_openssl() {
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_crypto_strings();
	OpenSSL_add_ssl_algorithms();
	PINFO("SSL Initialized.");
}

void cleanup_openssl() {
	EVP_cleanup();
	PINFO("SSL Cleaned.");
}

SSL_CTX* create_context(bool  server ) {
	const SSL_METHOD *method;
	SSL_CTX *ctx;

//	method = server ? SSLv23_server_method(): SSLv23_client_method();
	method = TLS_method();

	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	} else {
		PINFO("SSL Context Created.");
	}
	return ctx;
}

void configure_context(SSL_CTX *ctx, const char cert_loc[],
		const char key_loc[]) {
	SSL_CTX_set_ecdh_auto(ctx, 1);
//	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

	/* Set the key and cert */
	if (SSL_CTX_use_certificate_file(ctx, cert_loc, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, key_loc, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	PINFO("SSL Context Configured.");
}

