#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include <string>
#include <map>
#include <set>

#define PORT "5678"
#define STR_PATH "some_path.log"
#define N_CON 10


int load_strings(const char* path, std::set<std::string>& storage);
int dump_strings(const char* path, std::set<std::string>& storage);

int handle_request(int fd, std::string msg);

int set_con_name(int fd, std::string name);
int save_string(std::string str);
int remove_string(std::string str);
std::set<std::string> get_strings();

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
  struct addrinfo* ai, * p;

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

  for(p = ai; p != NULL; p = p->ai_next) {
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

  if(listen(listenerfd, N_CON) == -1) {
    perror("failed to listen");
    return 3;
  }

  FD_SET(listenerfd, &master_set);

  fdmax = listenerfd;

  while(true) {
    recent_set = master_set;
    if (select(fdmax + 1, &recent_set, NULL, NULL, NULL) == -1) {
      perror("failed to select");
      return 4;
    }

    for(i = 0; i <= fdmax; ++i) {
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
          //TODO
        }
      }
    } //END SELECT
  } //END WHILE
  return 0;
}
