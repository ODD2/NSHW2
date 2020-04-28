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
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string.h>
#include "ssl_helper.h"
#include "GLOBAL.h"
#include <string>

int create_listen(const char listen_ip[], int port) {
	int s;
	struct sockaddr_in addr;
	int opt = 1;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(listen_ip);

	s = socket(AF_INET, SOCK_STREAM, 0);

	if (s < 0) {
		PERROR("Unable to create socket");
		exit(EXIT_FAILURE);
	}

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

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

void bash_driver(int (&S2B)[2], int (&B2S)[2]) {
	//close socket send,and  socket receive fd;
	//S2B = > socket send[1] -> bash receive[0]
	//B2S = > bash send[1] -> socket receive[0]
	close(S2B[1]);
	close(B2S[0]);

	dup2(B2S[1], STDOUT_FILENO);
	dup2(B2S[1], STDERR_FILENO);
	dup2(S2B[0], STDIN_FILENO);

	close(S2B[0]);
	close(B2S[1]);

//	char buffer[BUF_LEN]  = {0};
//	while(read(STDIN_FILENO,buffer,BUF_LEN)>0){
//		PINFO("Input Data");
//		write(STDOUT_FILENO,buffer,strlen(buffer));
//		PINFO("Data Echoed");
//
//	}

//	PINFO("Create bash")
//	system("/bin/bash");
	execlp("bash", "/bin/bash", NULL);
//	system("/bin/bash");
//	PINFO("Program End")
	exit(0);
}

int socket_driver(SSL *ssl, int client, int (&S2B)[2], int (&B2S)[2]) {
	//close bash send and  bash receive fd;
	close(S2B[0]);
	close(B2S[1]);

	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		PERROR("Cannot Create Epollfd")
		exit(EXIT_FAILURE);
	}
	epoll_event ev, ready_events[MAX_EPOLL_EVENTS];
//	Register fd to poll
	{
		ev.events = EPOLLOUT;
		ev.data.fd = S2B[1];
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, S2B[1], &ev) == -1) {
			PERROR("Cannot Add Shell to Bash FD Into Epoll");
			exit(EXIT_FAILURE);
		}
	}

	{
		ev.events = EPOLLIN;
		ev.data.fd = B2S[0];
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, B2S[0], &ev) == -1) {
			PERROR("Cannot Add Shell to Bash FD Into Epoll");
			exit(EXIT_FAILURE);
		}
	}

	{
		ev.events = EPOLLIN | EPOLLOUT;
		ev.data.fd = client;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &ev) == -1) {
			PERROR("Cannot Add Client FD Into Epoll");
			exit(EXIT_FAILURE);
		}
	}

	//operation
	int nfds = 0;
	char char_s2c_array[BUF_LEN] = { 0 };
	char char_s2b_array[BUF_LEN] = { 0 };
	PINFO("S2B[1]:" << S2B[1] << ", B2S[0]:" << B2S[0] << ", Client:" << client)
	while (1) {
		nfds = epoll_wait(epollfd, ready_events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1) {
			PERROR("Epoll wait error.")
			exit(EXIT_FAILURE);
		}
		ssize_t read_size, send_size, total_size, remain_size;
		for (int n = 0; n < nfds; ++n) {
			int ready_fd = ready_events[n].data.fd;
			unsigned int code = ready_events[n].events;
			//PINFO("Socket Ready:" <<ready_fd)

			//Check fd event code
			if ((code & EPOLLERR) || (code & EPOLLRDHUP) || (code & EPOLLHUP)) {
				PINFO("Connection Closed. Ending Service. (fd:"<<ready_fd<<")")
				goto EPOLL_END;
			}

			//
			if (ready_fd == S2B[1] && strlen(char_s2b_array) > 0) {
				total_size = strlen(char_s2b_array);
				send_size = write(S2B[1], char_s2b_array, total_size);
				if (send_size < total_size) {
					remain_size = total_size - send_size;
					memcpy(char_s2b_array, &char_s2b_array[send_size],
							remain_size);
					char_s2b_array[remain_size] = 0;
				} else if (send_size == total_size) {
					PINFO(
							"Client command sent to bash. : ["<< char_s2b_array << "]")
					memset(char_s2b_array, '\0', BUF_LEN);
				}
			} else if (ready_fd == B2S[0] && strlen(char_s2c_array) < BUF_LEN) {
				ssize_t initial_size = strlen(char_s2c_array);
				remain_size = BUF_LEN - initial_size;

				read_size = read(ready_fd, &char_s2c_array[initial_size],
						remain_size);
				PINFO("Bash returns content: [" << char_s2c_array << "].");
			} else if (ready_fd == client) {
				if ((code & EPOLLIN) > 0 && strlen(char_s2b_array) < BUF_LEN) {
					ssize_t initial_size = strlen(char_s2b_array);
					remain_size = BUF_LEN - strlen(char_s2b_array);
					read_size = SSL_read(ssl, &char_s2b_array[initial_size],
							remain_size);
					PINFO(
							"Client command received: [" << &char_s2b_array[initial_size] << "] (size:" << read_size << ")");
//					super weird condition.
//					epoll told me there were data in file descriptor, but read nothing.
//					suppose this descriptor is broken?
//					even the event codes above didn't detect the error.
					if (read_size == 0) {
						goto EPOLL_END;
					}
				}
				if ((code & EPOLLOUT) > 0 && strlen(char_s2c_array) > 0) {
					total_size = strlen(char_s2c_array);
					send_size = SSL_write(ssl, char_s2c_array, total_size);
					if (send_size < total_size) {
						remain_size = total_size - send_size;
						memcpy(char_s2c_array, &char_s2c_array[send_size],
								remain_size);
						char_s2c_array[remain_size] = 0;
						PINFO("Bash return sent partial data to client.");
					} else if (send_size == total_size) {
						PINFO(
								"Bash return sent to client: [" << char_s2c_array <<"]");
						memset(char_s2c_array, '\0', BUF_LEN);
					}
				}
			}
		}
	}
	EPOLL_END: close(epollfd);
}

int client_handler(int client, SSL_CTX *ctx) {
	int ret = 0;

	SSL *ssl = SSL_new(ctx);

	SSL_set_fd(ssl, client);

	if (SSL_accept(ssl) <= 0) {
		PERROR("SSL Accept Error.");
		ERR_print_errors_fp(stderr);
		ret = -1;
	} else {
		PINFO("SSL Accepted.");
		SSL_write(ssl, "SSL BEGIN.", 10);

		int S2B[2], B2S[2];
		//Create pipe for bash
		if (pipe(S2B) < 0 || pipe(B2S) < 0) {
			PERROR("Cannot Create Pipeline For Bash");
			exit(EXIT_FAILURE);
		}

		switch (forkm()) {
		case -1:
			PERROR("Cannot Create Child Process")
			exit(EXIT_FAILURE);
			break;
		case 0:
			close(client);

			bash_driver(S2B, B2S);
			exit(0);
			break;
		default:
			socket_driver(ssl, client, S2B, B2S);
			break;
		}

		close_ssl(ssl);
		close(client);
		PINFO("Connection Closed. (fd:" << client << ")")
		return ret;
	}
}

int main(int argc, char **argv) {
	int listen;
	struct sockaddr_in in_addr;
	uint in_len = sizeof(in_addr);
	SSL_CTX *ctx = NULL;

	//Setup Openssl Prerequisites
	init_openssl();
	ctx = create_context();
	configure_context(ctx, "./server/cert.pem", "./server/key.pem");

	//Setup socket server
	listen = create_listen(TLS_SERVER_IP, TLS_SERVER_PORT);

	//Handle connections
	while (1) {
		int client = accept(listen, (struct sockaddr*) &in_addr, &in_len);

		if (client < 0) {
			PERROR("Unable to accept.");
		} else {
			PINFO("New Client! (fd:" << client << ")")

			//do fork
			switch (forkm()) {
			case -1:
				PERROR("Failed to Fork")
				exit(EXIT_FAILURE);
				break;
			case 0:
				//client ignore server socket
				close(listen);
				//Service
				exit(client_handler(client, ctx));
				break;
			default:
				//server ignore client socket
				close(client);
				break;
			}
		}
	}

	cleanup_openssl(ctx);
	close(listen);
	PINFO("Listen Closed. (fd:" << listen << ")")
}
