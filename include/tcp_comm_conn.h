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
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "5678"

#define MAX_NAME 64
#define MAX_SIZE 256
#define MSG_SIZE_LEN sizeof(uint32_t)

#define HEX_MASK 0xff

#define CMD_NAME "0"
#define CMD_SAVE "1"
#define CMD_REM  "2"
#define CMD_LIST "3"

int get_addr_info(const char* address, const char* port, struct addrinfo** ai);


int set_sock_keepalive_opt(int fd, const int *keep_idle, const int *probes_cnt, const int *probes_intvl);

void serialize_uint(uint32_t value, uint8_t* buf);

uint32_t deserialize_uint(uint8_t* buf);

int send_string(int fd, const char* buf, size_t size, int flags);

int recv_string(int fd, char* buf, uint32_t* len, int flags);


#ifdef __cplusplus
}
#endif
