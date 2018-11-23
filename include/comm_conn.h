#pragma once


#ifdef __cplusplus
extern "C" {
#endif
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

#define PORT "5678"

#define MAX_NAME 64
#define MAX_SIZE 256

#define CMD_NAME "0"
#define CMD_SAVE "1"
#define CMD_REM  "2"
#define CMD_LIST "3"

#define safe_close(fd) close(fd); (fd) = -1

int get_addr_info(const char* address, const char* port, struct addrinfo** ai);


int set_sock_keepalive_opt(int fd, const int *keep_idle, const int *probes_cnt, const int *probes_intvl);

#ifdef __cplusplus
}
#endif
