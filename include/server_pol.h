#pragma once

#include "tcp_server.h"
#include <poll.h>

namespace server_poll {

class TCP_Server_Pol : public tcp_server::TCP_Server {
 private:
  const int max_conn;
  const int wait_time;
  int cur_size = 0;
  struct pollfd* ufds;
  
 protected:
  virtual void init();

  virtual int wait_for_connection();

  virtual int add_new_connection(int newfd);
  virtual void remove_connection(const int fd);

  virtual int idx_to_fd(const int idx) const;
  virtual int get_next_index(int& i, int& cur_num, const int total_num);

 public:
  TCP_Server_Pol(const std::string& _port,
                 const int _max_conn = 64,
                 const int wait_time = 60 * 1000,
                 const std::string& _str_path = "tcp_strings.txt",
                 const int _n_con = 10);
  ~TCP_Server_Pol();
};
}  // namespace server_poll