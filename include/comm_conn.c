#include "comm_conn.h"

int get_addr_info(const char* address, const char* port, struct addrinfo** ai) 
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (address == NULL) {
    hints.ai_flags = AI_PASSIVE;
  }
  return getaddrinfo(address, PORT, &hints, ai);
}


int set_sock_keepalive_opt(int fd, const int *keep_idle, const int *probes_cnt, const int *probes_intvl) 
{
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(true)) == -1) {
    perror("setsockopt(SO_KEEPALIVE)");
    return -1;
  } 
  if (keep_idle != NULL) {
    if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, keep_idle, sizeof(*keep_idle)) == -1) {
      perror("setsockopt(TCP_KEEPIDLE)");
      return -2;
    }
  }

  if (probes_cnt != NULL) {
    if (setsockopt(fd, SOL_TCP, TCP_KEEPCNT, probes_cnt, sizeof(*probes_cnt)) == -1) {
      perror("setsockopt(TCP_KEEPIDLE)");
      return -3;
    }
  }

  if (probes_intvl != NULL) {
    if (setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, probes_intvl, sizeof(*probes_intvl)) == -1) {
      perror("setsockopt(TCP_KEEPIDLE)");
      return -4;
    }
  }

  return 0;
}