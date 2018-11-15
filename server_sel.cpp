#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include <fstream>

#define PORT "5678"
#define STR_PATH "some_path.log"
#define N_CON 10

int load_strings(const char* path, std::vector<std::string>& storage);
int dump_strings(const char* path, std::vector<std::string>& storage);

int handle_request(int fd, std::string& msg, std::vector<std::string>& storage,
                   std::map<int, std::string>& names);
int parse_msg(std::string& msg);

int set_con_name(int fd, const std::string& name,
                 std::map<int, std::string> names);
int save_string(const std::string& str, const std::string& name, std::vector<std::string>& storage);
int remove_string(const std::string& str, std::vector<std::string>& storage);
std::vector<std::string> get_strings();

int main() {
  fd_set master_set;
  fd_set recent_set;
  int fdmax;

  int listenerfd;
  int newfd;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;

  char buf[256];
  int nbytes;

  int yes = 1;
  int i, rval;

  struct timeval tv;
  tv.tv_sec = 60;
  tv.tv_usec = 0;

  struct addrinfo hints;
  struct addrinfo *ai, *p;

  FD_ZERO(&master_set);
  FD_ZERO(&recent_set);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if ((rval = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "SERVER_SEL ERROR: %s\n", gai_strerror(rval));
    return 1;
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    listenerfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listenerfd < 0) {
      continue;
    }

    setsockopt(listenerfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listenerfd, p->ai_addr, p->ai_addrlen) < 0) {
      close(listenerfd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "SERVER_SEL ERROR: failed to bind\n");
    return 2;
  }

  freeaddrinfo(ai);

  if (listen(listenerfd, N_CON) == -1) {
    perror("failed to listen");
    return 3;
  }

  FD_SET(listenerfd, &master_set);

  fdmax = listenerfd;

  while (true) {
    recent_set = master_set;
    if (select(fdmax + 1, &recent_set, NULL, NULL, NULL) == -1) {
      perror("failed to select");
      return 4;
    }

    for (i = 0; i <= fdmax; ++i) {
      if (FD_ISSET(i, &recent_set)) {
        if (i == listenerfd) {
          addrlen = sizeof(remoteaddr);
          newfd = accept(listenerfd, (struct sockaddr*)&remoteaddr, &addrlen);

          if (newfd == -1) {
            perror("failed to accept");
          } else {
            FD_SET(newfd, &master_set);
            if (newfd > fdmax) {
              fdmax = newfd;
            }
            printf("some1 connected on socket %d\n", newfd);
          }
        } else {
          if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
            if (nbytes == 0) {
              printf("connection on socket %d was closed\n", i);
            } else {
              perror("failed to recv");
            }
            close(i);
            FD_CLR(i, &master_set);
          } else {
          }
          // TODO
        }
      }
    }  // END SELECT
  }    // END WHILE
  return 0;
}

int load_strings(const char* path, std::vector<std::string>& storage) {
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
            "created",
            path);
    // fprintf(stderr, "SERVER_SEL ERROR: cannot open the file %s", path);
    return 1;
  }
}

int dump_strings(const char* path, const std::vector<std::string>& storage) {
  std::ofstream outfile(path);
  if (outfile.is_open()) {
    for (int i = 0; i < storage.size(); ++i) {
      outfile << storage[i] << std::endl;
    }
    return 0;
  } else {
    fprintf(stderr, "SERVER_SEL ERROR: cannot open the file %s", path);
    return 1;
  }
}

int handle_request(int fd, std::string& msg, std::vector<std::string>& storage,
                   std::map<int, std::string>& names) {
  int command = parse_msg(msg);
  switch (command) {
    case 0: {
      set_con_name(fd, msg, names);
      break;
    }
    case 1: {
      save_string(msg, names[fd], storage);
      break;
    }
    case 2: {
      remove_string(msg, storage);
      break;
    }
    case 3: {
      get_strings();
      break;
    }
    default: {
      fprintf(stderr, "unrecognized command\n");
      return 1;
    }
  }
}

int parse_msg(std::string& msg) {
  int rval = -1;
  if (msg.size() > 0) {
    rval = msg[0];
  }
  if (msg.size() > 2) {
    msg = msg.substr(2);
  }
  return rval;
}

int save_string(const std::string& msg, const std::string& name, std::vector<std::string> storage) {
  storage.push_back(msg + ", by " + name);
}
