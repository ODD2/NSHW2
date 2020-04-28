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
#include <sys/epoll.h>
#include <iostream>
#include <thread>
#include <string>

using namespace std;

int connect_socket(const char server_ip[], int port) {
	int s;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(server_ip);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		PERROR("Unable to create socket");
		exit(EXIT_FAILURE);
	}

	if (connect(s, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		PERROR("Connection error");
		exit(EXIT_FAILURE);
	}

	return s;
}



int main(int argc, char **argv) {
	int con_fd = 0;
	SSL_CTX *ctx;
	SSL *ssl;
	char buffer[BUF_LEN] = { 0 };

	init_openssl();
	ctx = create_context();
	configure_context(ctx, "./client/cert.pem", "./client/key.pem");


	con_fd = connect_socket(TLS_SERVER_IP, TLS_SERVER_PORT);
	PINFO("Connected to Server.")

	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, con_fd);

	if (SSL_connect(ssl) != 1) {
		PERROR("SSL Connection Failed")
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
	PINFO("SSL Connection Success");

//	SSL_read(ssl, buffer, BUF_LEN);
//	PINFO("SSL reads: \n[" << buffer << "]");

	int epollfd = epoll_create1(0);
	epoll_event ev, read_evs[MAX_EPOLL_EVENTS];

	while(1 ){
		if(!(cin >> buffer))
		{
			PERROR("Cannot get command from stdin.")
			exit(EXIT_FAILURE);
		}

//		if(strlen(buffer)<BUF_LEN-2){
//			buffer[strlen(buffer)] = '\n';
//			buffer[strlen(buffer)] = '\0';
//		}

		buffer[strlen(buffer)] = '\n';
		if(SSL_write(ssl,buffer,BUF_LEN)<0){
			PERROR("Connection Closed(Writing).")
			exit(EXIT_FAILURE);
		}

		if(SSL_read(ssl,buffer,BUF_LEN)<0){
			PERROR("Connection Closed(Reading).")
			exit(EXIT_FAILURE);
		}
		cout << buffer;
	}

	close_ssl(ssl);

	cleanup_openssl(ctx);

	close(con_fd);
	PINFO("Connection Closed. (fd:" << con_fd << ")")
}
