#pragma once

#include "tcp_server.h"

namespace server_select {

class TCP_Server_Sel : public tcp_server::TCP_Server {
 private:
  fd_set master_set, recent_set;
  struct timeval master_tv, recent_tv;
  int fdmax = 0;

 protected:
  virtual void init();

  virtual int wait_for_connection();

  virtual int add_new_connection(int newfd);
  virtual void remove_connection(const int fd);

  virtual int idx_to_fd(int idx) const;
  virtual int get_next_index(int& i, int& cur_num, const int total_num);

 public:
  TCP_Server_Sel(const std::string& _port, const std::string& _str_path = "tcp_strings.txt",
                 const int _n_con = 10);
};
}  // namespace server_select