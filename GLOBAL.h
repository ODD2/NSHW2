/*
 * GLOBAL.h
 *
 *  Created on: Apr 27, 2020
 *      Author: xd
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_
#define DEBUG

#include <stdio.h>
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;


#define PERROR( x ) cerr << __PRETTY_FUNCTION__ << ": " << x   << endl;

#ifdef DEBUG
#define PINFO( x ) cout << __PRETTY_FUNCTION__ <<  ": " <<  x   << endl;
#endif

#define BUF_LEN 1024

#define TLS_SERVER_PORT 5000
#define TLS_SERVER_IP "127.0.0.1"

#define MAX_EPOLL_EVENTS 20

int forkm();



#endif /* GLOBAL_H_ */
