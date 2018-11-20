#include "tcp_client.h"

int get_conn_socket(const char* address, const char* port) {
  struct addrinfo *ai, *p;

  int rv = 0;
  if ((rv = get_addr_info(address, port, &ai) != 0) {
    return rv;
  }

  int connected_fd = -1;
  for(p = ai; p != NULL; p = p->ai_next) {
    if ((connected_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (connect(connected_fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(connected_fd);
      perror("connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "cliend: failed to connect\n");
    return -1;
  }

  freeaddrinfo(ai);
  
  return connected_fd;
}