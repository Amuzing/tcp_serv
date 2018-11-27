#include "tcp_server.h"

const char* welcome_buf = 
      "Hello. To add a string to the server, type 1 and the string as a "
      "param.\n"
      "To remove the string from the server, type 2 and the string as a "
      "param.\n"
      "To list all the strings and the ones who added them, type 3.\n";

int get_listening_socket(const char* port, const int n_con) 
{
  struct addrinfo *ai, *p;

  int rv = 0;
  if ((rv = get_addr_info(NULL, port, &ai)) != 0) {
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

  return fd;
}

int accept_new_connection(const int listening_fd) {
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  addrlen = sizeof(remoteaddr);
  return accept(listening_fd, (struct sockaddr*)&remoteaddr, &addrlen);
}

int load_strings(const char* path, std::list<std::string>& storage) {
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
            path);
    // fprintf(stderr, "SERVER_SEL ERROR: cannot open the file %s", path);
    return 1;
  }
}

int dump_strings(const char* path, const std::list<std::string>& storage) {
  std::ofstream outfile(path);
  if (outfile.is_open()) {
    for (auto it = std::begin(storage); it != std::end(storage); ++it) {
      outfile << *it << std::endl;
    }
    return 0;
  } else {
    fprintf(stderr, "SERVER_SEL ERROR: cannot open the file %s\n", path);
    return 1;
  }
}

int set_con_name(int fd, const std::string& name,
                 std::map<int, std::string>& names) {
  names[fd] = name;
  return 0;
}

CMD parse_msg(std::string& msg) {
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

int save_string(const std::string& str, const std::string& name,
                std::list<std::string>& storage) {
  storage.push_back(str + ", by " + name);
  return 0;
}

int remove_string(const std::string& str, std::list<std::string>& storage) {
  for (auto it = std::begin(storage); it != std::end(storage);) {
    if (it->find(str) != std::string::npos) {
      auto next_it = std::next(it);
      storage.erase(it);
      it = next_it;
    } else {
      ++it;
    }
  }
  return 0;
}

int print_strings(int fd, const std::list<std::string>& storage) {
  
  std::string delims = "**************************\n";
  std::string output_str = delims;
  for(auto it = std::begin(storage); it != std::end(storage); ++it) {
    output_str += (*it) + "\n";
    /*printf("sending: %s", it->c_str());
    if(send(fd, (*it + "\n").c_str(), NT_TO_STR_CR_SIZE(it->size()), 0) == -1) {
      perror("failed to send");
    }*/
  } 
  output_str += delims + "\n";
  //send(fd, delims, strlen(delims), 0);
  //send(fd, END_PRINT, sizeof(END_PRINT), 0);
  printf("Sending a string %lu bytes long\n", std::size(output_str));
  return send_string(fd, output_str.c_str(), std::size(output_str)); 
}

int handle_request(int fd, std::string& msg, std::list<std::string>& storage,
                   std::map<int, std::string>& names) {
  CMD command = parse_msg(msg);
  switch (command) {
    case '0': {
      set_con_name(fd, msg, names);
      break;
    }
    case '1': {
      save_string(msg, names[fd], storage);
      break;
    }
    case '2': {
      remove_string(msg, storage);
      break;
    }
    case '3': {
      print_strings(fd, storage);
      break;
    }
    default: {
      fprintf(stderr, "unrecognized command %d\n", command);
      return 1;
    }
  }
  return 0;
}

void sighup_handler(int sig) {
  extern std::list<std::string> str_storage;
  int fd = open(STR_PATH, O_WRONLY | O_CREAT | O_TRUNC, 777);
  if (fd != -1) {
    for(auto it = std::begin(str_storage); it != std::end(str_storage); ++it) {
      write(fd, (*it + "\n").c_str(), NT_TO_STR_CR_SIZE(it->size()));
    }
    close(fd);
  } else {
    perror("failed to open a file for dumping data");
  }
}

int define_sighup_handler() {
  struct sigaction hup_sa;
  hup_sa.sa_handler = sighup_handler;
  hup_sa.sa_flags = 0;
  sigemptyset(&hup_sa.sa_mask);

  if (sigaction(SIGHUP, &hup_sa, NULL) == -1) {
    perror("sigaction");
    return 5;
  }
  return 0;
}
