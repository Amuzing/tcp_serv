#include "tcp_server.h"

std::list<std::string> str_storage;


int main() {
  fd_set master_set;
  fd_set recent_set;
  int fdmax;

  int newfd;

  char buf[256];
  
  int nbytes;

  define_sighup_handler();

  load_strings(STR_PATH, str_storage);

  std::map<int, std::string> names;

  FD_ZERO(&master_set);
  FD_ZERO(&recent_set);

  int listening_fd = -1;
  if((listening_fd = get_listening_socket(PORT, N_CON)) < 0) {
    //TODO
    printf(":(\n");
  }

  FD_SET(listening_fd, &master_set);

  struct timeval master_tv;
  master_tv.tv_sec = 60;
  master_tv.tv_usec = 0;
  struct timeval recent_tv;
  recent_tv.tv_sec = 60;
  recent_tv.tv_usec = 0;
  fdmax = listening_fd;
  while (true) {
    recent_set = master_set;
    recent_tv = master_tv;
    if (select(fdmax + 1, &recent_set, NULL, NULL, &recent_tv) == -1) {
      perror("failed to select");
      return 4;
    }

    for (int i = 0; i <= fdmax; ++i) {
      if (FD_ISSET(i, &recent_set)) {
        if (i == listening_fd) {
          newfd = accept_new_connection(listening_fd);

          if (newfd == -1) {
            perror("failed to accept");
          } else {
            // keep-alive
            int idle_time = 60;
            set_sock_keepalive_opt(newfd, &idle_time, NULL, NULL);

            FD_SET(newfd, &master_set);
            if (newfd > fdmax) {
              fdmax = newfd;
            }
            printf("some1 connected on socket %d\n", newfd);
            send(newfd, welcome_buf, strlen(welcome_buf), 0);
            printf("Welcome string was sent to socket %d.\n", newfd);
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
            printf("\n nbytes: %d, string from buf:%s!!!!\n", nbytes, buf);
            std::string request(buf, nbytes);
            handle_request(i, request, str_storage, names);
          }
          // TODO
        }
      }
    }  // END SELECT
  }    // END WHILE
  return 0;
}

