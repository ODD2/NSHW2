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
#include <iostream>
using namespace std;

int connect_socket(const char server_ip[], int port) {
	int s;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(server_ip);

	s = socket(AF_INET, SOCK_STREAM, 0);

	if (s < 0) {
		PERROR("Unable to create socket");
		exit(EXIT_FAILURE);
	}

	int c = connect(s, (struct sockaddr*) &addr, sizeof(addr));

	if (c == -1) {
		PERROR("Connection error");
		exit(EXIT_FAILURE);
	}

	return c;
}

int main(int argc, char **argv) {
	int con_fd = 0;
	SSL_CTX *ctx;
	SSL *ssl;
	char buffer[BUF_LEN] = { 0 };

	init_openssl();
	ctx = create_context(false);
	configure_context(ctx, "./client/cert.pem", "./client/key.pem");

	con_fd = connect_socket(TLS_SERVER_IP, TLS_SERVER_PORT);
	PINFO("Connected to Server.")

	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, con_fd);

	if (SSL_connect(ssl)  != 1) {
		PERROR("SSL Connection Failed")
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
	PINFO("SSL Connection Success");

	SSL_read(ssl, buffer, BUF_LEN);
	PINFO("SSL reads: [" << buffer << "]");

	SSL_shutdown(ssl);
	SSL_free(ssl);

	close(con_fd);

	SSL_CTX_free(ctx);

	cleanup_openssl();
}
