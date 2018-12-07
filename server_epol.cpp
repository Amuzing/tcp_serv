#include "server_epol.h"

int main() {
  server_epoll::TCP_Server_Epol serv("5678");
  serv.main();
  return 0;
}

namespace server_epoll {
TCP_Server_Epol::TCP_Server_Epol(const std::string& _port, const int _max_conn,
                                 const int _wait_time,
                                 const std::string& _str_path, const int _n_con)
    : TCP_Server(_port, _str_path, _n_con),
      max_conn(_max_conn),
      wait_time(_wait_time) {
  init();
}

TCP_Server_Epol::~TCP_Server_Epol() {
  printf("Freeing memory before deleting the object.\n");
  free(events);
}

void TCP_Server_Epol::init() {
  load_strings();

  set_listening_socket(get_port(), get_n_con());

  events = (struct epoll_event*)malloc(max_conn * sizeof(struct epoll_event));

  efd = epoll_create1(0);
  struct epoll_event new_event;
  new_event.data.fd = get_listening_fd();
  new_event.events = EPOLLIN;

  if (epoll_ctl(efd, EPOLL_CTL_ADD, get_listening_fd(), &new_event) == -1) {
    perror("epoll_ctl on listening socket: ");
  }
}

int TCP_Server_Epol::wait_for_connection() {
  int rv = epoll_wait(efd, events, max_conn, wait_time);
  if (rv == -1) {
    throw;
  }
  return rv;
}

int TCP_Server_Epol::add_new_connection(int newfd) {
  struct epoll_event new_event;
  memset(&new_event, 0, sizeof(new_event));
  new_event.data.fd = newfd;
  new_event.events = EPOLLIN;
  if (epoll_ctl(efd, EPOLL_CTL_ADD, newfd, &new_event) == -1) {
    perror("epoll_ctl on a new connection: ");
    close(newfd);
    return -1;
  }
  return 0;
}

void TCP_Server_Epol::remove_connection(int idx) {
  int fd = idx_to_fd(idx);
  if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL) == -1) {
    perror("epoll_ctl on removing connection: ");
  }
  close(fd); 
}

int TCP_Server_Epol::get_next_index(int& i, int& cur_num, const int total_num) {
  for(; i < total_num; ++i) {
    printf("Checking events.\n");
    ++cur_num;
    return i++;
  }
  if (cur_num == total_num) {
    return -1;
  } else {
    return -2;
  }
}

int TCP_Server_Epol::idx_to_fd(const int idx) const {
  return events[idx].data.fd;
}

}  // namespace server_epoll