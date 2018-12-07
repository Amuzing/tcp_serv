#include "tcp_server.h"
#include <iostream>

namespace tcp_server {

TCP_Server::TCP_Server(const std::string& _port, const int _n_con)
    : port(_port), n_con(_n_con), storage({}), names({}) {}

TCP_Server::TCP_Server(const std::string& _port, const std::string& _str_path,
                       const int _n_con)
    : port(_port), path(_str_path), n_con(_n_con), storage({}), names({}) {}

int TCP_Server::set_listening_socket(const std::string& _port,
                                     const int n_con) {
  struct addrinfo *ai, *p;

  int rv = 0;
  if ((rv = get_addr_info(NULL, _port.c_str(), &ai)) != 0) {
    return rv;
  }

  int fd = -1;
  for (p = ai; p != NULL; p = p->ai_next) {
    fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd < 0) {
      perror("socket");
      continue;
    }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(fd, p->ai_addr, p->ai_addrlen) < 0) {
      perror("bind");
      close(fd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "SERVER_SEL ERROR: failed to bind\n");
    return -201;
  }

  freeaddrinfo(ai);

  if (listen(fd, n_con) == -1) {
    perror("failed to listen");
    return -202;
  }
  listening_fd = fd;
  printf("Listening on socket %d.\n", fd);
  return rv;
}

int TCP_Server::load_strings() {
  std::ifstream infile(path);
  std::string line;
  if (infile.is_open()) {
    while (std::getline(infile, line)) {
      storage.push_back(line);
    }
    printf("strings loaded\n");
    return 0;
  } else {
    fprintf(stderr,
            "SERVER_SEL ERROR: cannot open the file %s, the new one will be "
            "created\n",
            path.c_str());
    // fprintf(stderr, "SERVER_SEL ERROR: cannot open the file %s", path);
    return 1;
  }
}

int TCP_Server::dump_strings() {
  std::ofstream outfile(path);
  if (outfile.is_open()) {
    for (const auto& str : storage) {
      outfile << str << std::endl;
    }
    return 0;
  } else {
    fprintf(stderr, "SERVER_SEL ERROR: cannot open the file %s\n", path.c_str());
    return 1;
  }
}

int TCP_Server::accept_new_connection() {
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  addrlen = sizeof(remoteaddr);
  return accept(listening_fd, (struct sockaddr*)&remoteaddr, &addrlen);
}

void TCP_Server::set_con_name(int fd, const std::string& name) {
  names[fd] = name;
}

CMD TCP_Server::parse_msg(std::string& msg) {
  printf("string: %s size: %lu\n", msg.c_str(), msg.size());
  CMD rval = '\0';
  if (msg.size() > 0) {
    rval = msg[0];
  }
  if (msg.size() > 1) {
    msg = msg.substr(1);
  }
  return rval;
}

void TCP_Server::save_string(const std::string& str, const std::string& name) {
  storage.push_back(str + ", by " + name);
}

void TCP_Server::remove_string(const std::string& str) {
  for (auto it = std::begin(storage); it != std::end(storage);) {
    if (it->find(str) != std::string::npos) {
      it = storage.erase(it);
    } else {
      ++it;
    }
  }
}

bool TCP_Server::is_listening_socket(const int idx) const {
  return idx_to_fd(idx) == get_listening_fd();
}

int TCP_Server::print_strings(int fd) const {
  const std::string delims = "**************************\n";
  std::string output_str = delims;
  for (const auto& str : storage) {
    output_str += str + "\n";
  }
  output_str += delims + "\n";
  printf("Sending a string %lu bytes long\n", std::size(output_str));
  return send_string(fd, output_str.c_str(), std::size(output_str), MSG_DONTWAIT);
}

int TCP_Server::handle_request(const int fd, std::string& msg) {
  printf("Parsing a command and cutting message.\n");
  CMD command = parse_msg(msg);
  printf("Proceeding with a command %c.\n", command);
  switch (command) {
    case '0': {
      //printf("Setting desc %d to %");
      std::cout << "Setting desc " << fd << " to " << msg << "."
                << std::endl;
      set_con_name(fd, msg);
      std::cout << "Done." << std::endl;
      break;
    }
    case '1': {
      std::cout << "Saving msg: " << msg << " by user  " << names[fd] << " ."
                << std::endl;
      save_string(msg, names[fd]);
      std::cout << "Done." << std::endl;
      break;
    }
    case '2': {
      std::cout << "Removing msg: " << msg << " ." << std::endl;
      remove_string(msg);
      std::cout << "Done." << std::endl;
      break;
    }
    case '3': {
      std::cout << "Printing msgs to desc " << fd << " ." << std::endl;
      print_strings(fd);
      std::cout << "Done." << std::endl;
      break;
    }
    default: {
      fprintf(stderr, "unrecognized command %d\n", command);
      return 1;
    }
  }
  return 0;
}

const std::string TCP_Server::get_welcome_string() const {
  const std::string temp =
      "Hello. To add a string to the server, type 1 and the string as a "
      "param.\n"
      "To remove the string from the server, type 2 and the string as a "
      "param.\n"
      "To list all the strings and the ones who added them, type 3.\n";
  return temp;
}

int TCP_Server::handle_events(const int rv) {
  printf("Handle events.\n");
  int i = 0;
  int cur_num = 0;
  int idx = -1;
  do {
    printf("Getting next index.\n");
    idx = get_next_index(i, cur_num, rv);
    //printf("Idx is %d.\n", idx);
    if (idx == -1) {
      break;
    } else if (idx == -2) {
      printf("Not all the incoming requests were handled.\n");
      return -1;
    }
    //printf("Idx is %d.\n", idx);
    if (is_listening_socket(idx)) {
      if (listening_socket_event() == -1) {
        perror("listening socket event: ");
      }
    } else {
      //printf("Idx is %d.\n", idx);
      if (nonlistening_socket_event(idx) == -1) {
        perror("nonlistening socket event: ");
      }
      //printf("Idx is %d.\n", idx);
    }
    //printf("Next iteration.\n");
    //printf("Next iteration, idx = %d...\n", idx);
  } while (idx >= 0);
  printf("Handle events ending.\n");
  return 0;
}

int TCP_Server::listening_socket_event() {
  int newfd = accept_new_connection();
  if (newfd == -1) {
    perror("accept_new_connection: ");
    return -1;
  }
  int idle_time = 60;
  set_sock_keepalive_opt(newfd, &idle_time, NULL, NULL);

  int rv = add_new_connection(newfd);
  if (rv == -1) {
    return -1;
  }
  printf("Someone connected on socket %d.\n", newfd);
  send_string(newfd, get_welcome_string().c_str(),
              strlen(get_welcome_string().c_str()), 0);
  printf("Welcome string was sent to socket %d.\n", newfd);
  printf("Listening socket event ending.\n");
  return 0;
}

int TCP_Server::nonlistening_socket_event(const int idx) {
  printf("Nonlistening socket event.\n");
  char* buf = NULL;
  int nbytes = 0;
  uint32_t len = 0;
  int fd = idx_to_fd(idx);
  if ((nbytes = recv_string(fd, &buf, &len, 0)) <= 0) {
    if (nbytes == 0) {
      printf("Connection on socket %d was closed.\n", fd);
    } else {
      perror("failed to recv: ");
      return -1;
    }
    remove_connection(idx);
  } else {
    printf("Creating a string from buffer %d bytes long.\n", nbytes);
    std::string request(buf, nbytes);
    free(buf);
    printf("Created a string and freed the memory.\n");
    std::cout << request << std::endl;
    handle_request(fd, request);
  }
  printf("Nonlistening socket event ending.\n");
  return 0;
}

int TCP_Server::main() {
  int rv = 0;
  while (true) {
    try {
      printf("Waiting for connections...\n");
      rv = wait_for_connection();
    } catch (...) {
      perror("wait_for_connection");
      return -1;
    }

    if (rv > 0) {
      handle_events(rv);
    } else if (rv == 0) {
      printf("Timeout expired... Waiting for the data...\n");
    } else {
      perror("wait_for_connection returned negative value: ");
    }
  }
  return 0;
}

/*void TCP_Server::sighup_handler(int sig) {
  extern std::list<std::string> str_storage;
  int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 777);
  if (fd != -1) {
    for (const auto& str : storage) {
      write(fd, (str + "\n").c_str(), str.size() + 1);
    }
    close(fd);
  } else {
    perror("failed to open a file for dumping data");
  }
}*/

/*int TCP_Server::define_sighup_handler() {
  struct sigaction hup_sa;
  hup_sa.sa_handler = _sighup_handler;
  hup_sa.sa_flags = 0;
  sigemptyset(&hup_sa.sa_mask);

  if (sigaction(SIGHUP, &hup_sa, NULL) == -1) {
    perror("sigaction");
    return 5;
  }
  return 0;
}*/

// deprecated
/*void _sighup_handler(int sig) {
  extern std::list<std::string> str_storage;
  int fd = open(, O_WRONLY | O_CREAT | O_TRUNC, 777);
  if (fd != -1) {
    for (const auto& str: storage) {
      write(fd, (str + "\n").c_str(), str.size() + 1);
    }
    close(fd);
  } else {
    perror("failed to open a file for dumping data");
  }
}*/

}  // namespace tcp_server
