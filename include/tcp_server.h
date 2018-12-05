#pragma once

#include "tcp_comm_conn.h"

#include <list> 
#include <map>
#include <string>

#include <fstream>
#include <iterator>

namespace tcp_server {

typedef char CMD;

class TCP_Server {
 private:
  const std::string port;
  const std::string path = "tcp_strings.txt";
  const int n_con;
  int listening_fd = -1;

  //enum CMD {NAME = 0, SAVE = 1, REM = 2, LIST = 3};

  std::list<std::string> storage;
  std::map<int, std::string> names;
  
 //protected:
 public:
  inline int get_listening_fd() const {
    return listening_fd;
  }

  inline std::string get_port() const {
    return port;
  }

  inline int get_n_con() const {
    return n_con;
  }

  inline std::list<std::string>& get_storage() {
    return storage;
  }

  virtual int set_listening_socket(const std::string& _port, const int n_con) final;
  virtual void init() = 0;

  
  //virtual int define_sighup_handler() final;

  virtual int load_strings() final;
  virtual int dump_strings() final;

  
  CMD parse_msg(std::string& msg);

  virtual void set_con_name(int fd, const std::string& name) final;
  virtual void save_string(const std::string& str, const std::string& name) final;
  virtual void remove_string(const std::string& str) final;
  virtual int print_strings(int fd) const final;

  int handle_request(int fd, std::string& msg);

  virtual const std::string get_welcome_string() const final;  

  virtual int wait_for_connection() = 0;

  virtual int listening_socket_event() final;
  virtual int nonlistening_socket_event(const int i) final;

  virtual int accept_new_connection() final;

  virtual bool is_listening_socket(const int idx) const final;
  virtual int get_next_index(int& i, int& cur_num,const int total_num) = 0;

  virtual int handle_events(const int rv) final;
  virtual int add_new_connection(int newfd) = 0;
  virtual void remove_connection(const int fd) = 0;

  virtual int idx_to_fd(const int idx) const = 0;

 public:
  TCP_Server(const std::string& _port, const int _n_con = 10);
  TCP_Server(const std::string& _port, const std::string& _str_path,
             const int _n_con = 10);
  //virtual ~TCP_Server();
  virtual int main() final;
  //virtual void sighup_handler(int sig) final;
};

}  // namespace tcp_serv


void sighup_handler(int sig);

int define_sighup_handler();

