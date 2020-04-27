//============================================================================
// Name        : NSHW2.cpp
// Author      : ODD2
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "ssl_helper.h"
#include "GLOBAL.h"

int create_listen(const char listen_ip[], int port) {
	int s;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(listen_ip);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		PERROR("Unable to create socket");
		exit(EXIT_FAILURE);
	}

	if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		PERROR("Unable to bind");
		exit(EXIT_FAILURE);
	}

	if (listen(s, 1) < 0) {
		PERROR("Unable to listen");
		exit(EXIT_FAILURE);
	}

	PINFO("Start Listening. (IP:" << listen_ip << ", PORT:" << port << ")")
	return s;
}

int main(int argc, char **argv) {
	int sock;
	SSL_CTX *ctx;

	init_openssl();
	ctx = create_context(true);

	configure_context(ctx, "./server/cert.pem", "./server/key.pem");

	sock = create_listen(TLS_SERVER_IP, TLS_SERVER_PORT);

	/* Handle connections */
	while (1) {
		struct sockaddr_in addr;
		uint len = sizeof(addr);
		SSL *ssl;
		const char reply[] = "test";

		int client = accept(sock, (struct sockaddr*) &addr, &len);
		if (client < 0) {
			PERROR("Unable to accept.");
			exit(EXIT_FAILURE);
		} else {
			PINFO("New Client! (fd:" << client << ")")
		}

		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, client);

		if (SSL_accept(ssl) <= 0) {
			PERROR("SSL Accept Error.");
			ERR_print_errors_fp(stderr);
		} else {
			PINFO("SSL Accepted.");
			SSL_write(ssl, reply, strlen(reply));
		}

		SSL_shutdown(ssl);
		PINFO("SSL Shutdown.")
		SSL_free(ssl);
		close(client);
		PINFO("Connection Closed. (fd:" << sock << ")")
	}

	close(sock);
	PINFO("Listen Closed. (fd:" << sock << ")")
	SSL_CTX_free(ctx);
	cleanup_openssl();
}
