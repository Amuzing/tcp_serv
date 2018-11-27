#pragma once

#include "tcp_comm_conn.h"

#include <list>
#include <map>
#include <string>

#include <iterator>
#include <fstream>

#ifdef __cplusplus
extern "C" {
#endif

#define STR_PATH "some_path.log"
#define N_CON 10
#define END_PRINT "$"
#define NT_TO_STR_CR_SIZE(x) ((x) + 1)

typedef char CMD;

extern const char* welcome_buf;

int get_listening_socket(const char* port, const int n_con);

int accept_new_connection(const int listening_fd);

int load_strings(const char* path, std::list<std::string>& storage);
int dump_strings(const char* path, std::list<std::string>& storage);

int handle_request(int fd, std::string& msg, std::list<std::string>& storage,
                   std::map<int, std::string>& names);
CMD parse_msg(std::string& msg);

int set_con_name(int fd, const std::string& name,
                 std::map<int, std::string>& names);
int save_string(const std::string& str, const std::string& name,
                std::list<std::string>& storage);
int remove_string(const std::string& str, std::list<std::string>& storage);
int print_strings(int fd, const std::list<std::string>& storage);


void sighup_handler(int sig);

int define_sighup_handler();

#ifdef __cplusplus
}
#endif
