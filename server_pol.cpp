#include <poll.h>
#include <sys/ioctl.h>
#include "tcp_server.h"

const int MAX_CONN = 63;

std::list<std::string> str_storage;

int main() {
  int newfd;

  char buf[256];

  int nbytes;

  define_sighup_handler();

  load_strings(STR_PATH, str_storage);

  std::map<int, std::string> names;

  int listening_fd = -1;
  if ((listening_fd = get_listening_socket(PORT, N_CON)) < 0) {
    // TODO
    printf(":(\n");
    exit(1);
  }

  char on = 1;
  int rv = 0;//ioctl(listening_fd, FIONBIO, (char*)&on);
  if (rv < 0) {
    perror("ioctl() failed");
    close(listening_fd);
    exit(-1);
  }

  struct pollfd ufds[MAX_CONN + 1];
  ufds[0].fd = listening_fd;
  ufds[0].events = POLLIN;
  printf("listening socket: %d\n", ufds[0].fd);
  int cur_size = 1;

  // init
  for (int i = 1; i <= MAX_CONN; ++i) {
    ufds[i].fd = -1;
  }

  const int wait_time = 60 * 1000;  // 60 sec
  rv = 0;
  while (true) {
    rv = poll(ufds, cur_size, -1);

    if (rv == -1) {
      perror("poll");
    } else if (rv == 0) {
      printf("Timeout. There were no data.\n");
    } else {
      for (int i = 0; i < cur_size; ++i) {
        if (ufds[i].revents & POLLIN) {
          if (i == 0) {  // event on a listening socket
            printf("Listening socket event on socket %d.\n", ufds[i].fd);
            newfd = accept_new_connection(listening_fd);
            if (newfd == -1) {
              perror("failed to accept");
            } else {
              // keep-alive
              int idle_time = 60;
              set_sock_keepalive_opt(newfd, &idle_time, NULL, NULL);

              // searching for the empty struct
              printf("Searching for the empty struct.\n");
              int j = 1;
              ufds[cur_size].fd = newfd;
              ufds[cur_size].events = POLLIN;
              ++cur_size;
              printf("some1 connected on socket %d\n", newfd);
              send(newfd, welcome_buf, strlen(welcome_buf), 0);
              printf("Welcome string was sent to socket %d.\n", newfd);
            }

          } else {  // event on a non listening socket
            printf("Non listening socket event on socket %d.\n", ufds[i].fd);
            if ((nbytes = recv(ufds[i].fd, buf, sizeof(buf), 0)) <= 0) {
              if (nbytes == 0) {
                printf("connection on socket %d was closed\n", ufds[i].fd);
                safe_close(ufds[i].fd);
                ufds[i].events = 0;
                ufds[i].revents = 0;
                memmove(&ufds[i], &ufds[cur_size - 1], sizeof(struct pollfd));
                --cur_size;
              } else {
                perror("failed to recv");
              }
            } else {
              printf("\n nbytes: %d, string from buf:%s!!!!\n", nbytes, buf);
              std::string request(buf, nbytes);
              handle_request(ufds[i].fd, request, str_storage, names);
            }
          }
        }
      }
    }
  }  // END WHILE
  return 0;
}
