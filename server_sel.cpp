#include "server_sel.h"

int main() {
  server_select::TCP_Server_Sel serv("5678");
  serv.main();
  return 0;
}

namespace server_select {

//deleg constr

TCP_Server_Sel::TCP_Server_Sel(const std::string& _port,
                               const std::string& _str_path, const int _n_con)
    : TCP_Server(_port, _str_path, _n_con) {
  master_tv.tv_sec = 60;
  master_tv.tv_usec = 0;
  init();
}

void TCP_Server_Sel::init() {
  load_strings();
  FD_ZERO(&master_set);
  FD_ZERO(&recent_set);
  set_listening_socket(get_port(), get_n_con());
  FD_SET(get_listening_fd(), &master_set);
  fdmax = get_listening_fd();
}

int TCP_Server_Sel::wait_for_connection() {
  recent_set = master_set;
  recent_tv = master_tv;
  int rv = select(fdmax + 1, &recent_set, NULL, NULL, &recent_tv);
  if (rv == -1) {
    throw;
  }
  return rv;
}

int TCP_Server_Sel::add_new_connection() {
  int newfd = accept_new_connection();
  if (newfd == -1) {
    perror("accept_new_connection: ");
    return -1;
  }
  int idle_time = 60;
  set_sock_keepalive_opt(newfd, &idle_time, NULL, NULL);

  FD_SET(newfd, &master_set);
  if (newfd > fdmax) {
    fdmax = newfd;
  }
  printf("Someone connected on socket %d.\n", newfd);
  send_string(newfd, get_welcome_string().c_str(),
              strlen(get_welcome_string().c_str()), 0);
  printf("Welcome string was sent to socket %d.\n", newfd);
  printf("Listening socket event ending.\n");
  return 0;
}

void TCP_Server_Sel::remove_connection(int fd) {
  close(fd);
  FD_CLR(fd, &master_set);
}

bool TCP_Server_Sel::is_listening_socket(const int fd) const {
  return fd == get_listening_fd();
}

int TCP_Server_Sel::get_next_index(int& i, int& cur_num, const int total_num) {
  for (; i <= fdmax; ++i) {
    printf("Checking ISSET.\n");
    if (FD_ISSET(i, &recent_set)) {
      ++cur_num;
      return i++;
    }
  }
  if (cur_num == total_num) {
    return -1;
  } else {
    return -2;
  }
}

int TCP_Server_Sel::listening_socket_event(const int i) {
  printf("Listening socket event.\n");
  return add_new_connection();
}

}  // namespace server_select
