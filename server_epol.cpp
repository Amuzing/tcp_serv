#include <sys/epoll.h>
#include "tcp_server.h"

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

  int efd = epoll_create1(0);

  struct epoll_event new_event;
  const int MAX_CONN = 64;

  struct epoll_event events[MAX_CONN];

  new_event.data.fd = listening_fd;
  new_event.events = EPOLLIN;

  if (epoll_ctl(efd, EPOLL_CTL_ADD, listening_fd, &new_event) == -1) {
    perror("epoll_ctl on listening socket ");
    return 1;
  }

  while (true) {
    int n = epoll_wait(efd, events, MAX_CONN, -1);

    if (n > 0) {
      for (int i = 0; i < n; ++i) {
        if (events[i].data.fd == listening_fd) {  // event on listening socket
          printf("Listening socket event on socket %d.\n", events[i].data.fd);
          newfd = accept_new_connection(listening_fd);
          if (newfd == -1) {
            perror("failed to accept");
          } else {
            // keep-alive
            int idle_time = 60;
            set_sock_keepalive_opt(newfd, &idle_time, NULL, NULL);
            memset(&new_event, 0, sizeof(new_event));
            new_event.data.fd = newfd;
            new_event.events = EPOLLIN;
            if (epoll_ctl(efd, EPOLL_CTL_ADD, newfd, &new_event) == -1) {
              perror("epoll_ctl on a new connection: ");
              close(newfd);
              newfd = -1;
              continue;
            }
            printf("some1 connected on socket %d\n", newfd);
            send(newfd, welcome_buf, strlen(welcome_buf), 0);
            printf("Welcome string was sent to socket %d.\n", newfd);
          }
        } else {  // event on non listening socket
          if ((nbytes = recv(events[i].data.fd, buf, sizeof(buf), 0)) <= 0) {
            if (nbytes == 0) {
              printf("connection on socket %d was closed\n", events[i].data.fd);
              if (epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, NULL) ==
                  -1) {
                perror("epoll_ctl on a new connection: ");
              }
              close(events[i].data.fd);
            } else {
              perror("failed to recv");
            }
          } else {
            printf("\n nbytes: %d, string from buf:%s!!!!\n", nbytes, buf);
            std::string request(buf, nbytes);
            handle_request(events[i].data.fd, request, str_storage, names);
          }
        }
      }
    } else if (n == 0) {
      printf("Timeout occured. Continue...\n");
      continue;
    } else {
      perror("epoll_wait: ");
    }
  }

  return 0;
}
