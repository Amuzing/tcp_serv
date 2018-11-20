#include "tcp_server.h"

int get_listening_socket(const char* port, const int n_con) {
  struct addrinfo *ai, *p;

  int rv = 0;
  if ((rv = get_addr_info(NULL, port, &ai) != 0) {
    return rv;
  }

  int listening_fd = -1;
  for (p = ai; p != NULL; p = p->ai_next) {
    listening_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listening_fd < 0) {
      perror("socket");
      continue;
    }
    setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(listening_fd, p->ai_addr, p->ai_addrlen) < 0) {
      perror("bind");
      close(listening_fd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "SERVER_SEL ERROR: failed to bind\n");
    return -1;
  }

  freeaddrinfo(ai);

  if (listen(listening_fd, N_CON) == -1) {
    perror("failed to listen");
    return -2;
  }

  return listening_fd;
}