#include "server_pol.h"

int main() { 
  server_poll::TCP_Server_Pol serv("5678");
  serv.main();
  return 0; 
}

namespace server_poll {

TCP_Server_Pol::TCP_Server_Pol(const std::string& _port, const int _max_conn,
                               const int _wait_time,
                               const std::string& _str_path, const int _n_con)
    : TCP_Server(_port, _str_path, _n_con),
      max_conn(_max_conn),
      wait_time(_wait_time) {
  init();
}

TCP_Server_Pol::~TCP_Server_Pol() {
  printf("Freeing memory before deleting the object.\n");
  free(ufds);
}

void TCP_Server_Pol::init() {
  load_strings();

  set_listening_socket(get_port(), get_n_con());

  ufds = (struct pollfd*)malloc(max_conn * sizeof(struct pollfd));
  ufds[0].fd = get_listening_fd();
  ufds[0].events = POLLIN;
  cur_size = 1;

  for (int i = 1; i < max_conn; ++i) {
    ufds[i].fd = -1;
  }
}

int TCP_Server_Pol::wait_for_connection() {
  int rv = poll(ufds, cur_size, wait_time);
  if (rv == -1) {
    throw;
  }
  return rv;
}

int TCP_Server_Pol::add_new_connection(int newfd) {
  printf("Searching for the empty struct.\n");
  if (cur_size >= max_conn) {
    fprintf(stderr, "No space for a new connection.\n");
    return -1;
  }
  ufds[cur_size].fd = newfd;
  ufds[cur_size].events = POLLIN;
  ufds[cur_size].revents = 0;
  ++cur_size;
  return 0;
}

void TCP_Server_Pol::remove_connection(int idx) {
  close(idx_to_fd(idx));
  memmove(&ufds[idx], &ufds[--cur_size], sizeof(struct pollfd));
}

int TCP_Server_Pol::get_next_index(int& i, int& cur_num, const int total_num) {
  for (; i < cur_size; ++i) {
    printf("Checking revents.\n");
    if (ufds[i].revents & POLLIN) {
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

int TCP_Server_Pol::idx_to_fd(const int idx) const { return ufds[idx].fd; }

}  // namespace server_poll