#pragma once

#include "comm_conn.h"
#include <stdlib.h>

int get_conn_socket(const char* address, const char* port);


bool handle_command(int fd, char* buf, size_t max_size);

int send_cmd_to_server(int fd, char* cmd, char* buf, size_t max_size);
int read_strings(int fd, char* buf, size_t max_size);

int send_msg(int fd, char* msg);

int send_name(int fd, char* name, size_t max_size);