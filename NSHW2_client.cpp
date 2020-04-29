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
#include <string.h>

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
	char buf_c2s[BUF_LEN] = { 0 };
	char buf_recv[BUF_LEN] = { 0 };

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
	SSL_read(ssl, buf_recv, BUF_LEN);
	cout << buf_recv << endl;

	int epollfd = epoll_create1(0), nfds = 0;
	epoll_event ev, ready_evs[MAX_EPOLL_EVENTS];

	{
		ev.events = EPOLLIN | EPOLLOUT;
		ev.data.fd = con_fd;
		epoll_ctl(epollfd, EPOLL_CTL_ADD, con_fd, &ev);
	}

	{
		ev.events = EPOLLIN;
		ev.data.fd = STDIN_FILENO;
		epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	}

	while (1) {
		nfds = epoll_wait(epollfd, ready_evs, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1) {
			PERROR("Epoll wait error.")
			break;
		}

		for (int n = 0; n < nfds; ++n) {
			int ready_fd = ready_evs[n].data.fd;
			unsigned int code = ready_evs[n].events;
			if (code & EPOLLERR || code & EPOLLRDHUP) {
				goto EPOLL_END;
			}

			if (ready_fd == con_fd) {
				if ((code & EPOLLIN)) {
					ssize_t recv_size = SSL_read(ssl, buf_recv, BUF_LEN-1);
					//					super weird condition.
					//					epoll told me there were data in file descriptor, but read nothing.
					//					suppose this descriptor is broken?
					//					even the event codes above didn't detect the error.
					if (recv_size == 0) {
						goto EPOLL_END;
					}
					else{
						buf_recv[recv_size] = '\0';
						cout << buf_recv;
					}
				}
				if ((code & EPOLLOUT) > 0 && strlen(buf_c2s) > 0) {
					ssize_t orig_size = strlen(buf_c2s);
					ssize_t send_size = SSL_write(ssl, buf_c2s, orig_size);
					if (orig_size < send_size) {
						ssize_t rem_size = orig_size - send_size;
						memcpy(buf_c2s, &buf_c2s[send_size], rem_size);
						buf_c2s[rem_size] = '\0';
					} else {
						memset(buf_c2s, '\0', BUF_LEN);
					}
				}
			} else if (ready_fd == STDIN_FILENO) {
				ssize_t orig_size = strlen(buf_c2s);
				ssize_t rem_size = BUF_LEN - orig_size;
				cin.getline(&buf_c2s[orig_size], rem_size - 1);
				buf_c2s[strlen(buf_c2s)] = '\n';
			}
		}
	}
	EPOLL_END:

	close_ssl(ssl);

	cleanup_openssl(ctx);

	close(con_fd);
	PINFO("Connection Closed. (fd:" << con_fd << ")")
}

