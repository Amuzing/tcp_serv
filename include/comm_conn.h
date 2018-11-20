#pragma once

#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef PORT
#define PORT "5678"
#endif

int get_addr_info(const char* address, const char* port, struct addrinfo** ai);



int set_sock_keepalive_opt(int fd, const void *keep_idle, const void *probes_cnt, const void *probes_intvl);
