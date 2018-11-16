#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "5678"
#define MAX_SIZE 256

bool some_func(int fd);

int main(int argc, char* argv[]) {
  int fd;
  struct addrinfo hints, *ai, *p;
  int nbytes;
  int yes = 1;
  int idle_time = 60;
  int rv;
  bool is_true = true;
  char* address = NULL;

  char buf[MAX_SIZE];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (argc == 2) {
    address = malloc(sizeof(char) * strlen(argv[1]));
    strcpy(address, argv[1]);
    memset(&hints.ai_flags, 0, sizeof(hints.ai_flags));
  }

  if (address != NULL) {
    printf("connected to %s\n", address);
  }

  if ((rv = getaddrinfo(address, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "CLIENT ERROR: %s\n", gai_strerror(rv));
    free(address);
    return 1;
  }
  free(address);

  for (p = ai; p != NULL; p = p->ai_next) {
    if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(fd);
      perror("connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "cliend: failed to connect\n");
    return 2;
  }

  freeaddrinfo(ai);

  // keep-alive
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
    perror("setsockopt(SO_KEEPALIVE)");
  }
  if (setsockopt(fd, IPPROTO_IP, TCP_KEEPIDLE, &idle_time, sizeof(idle_time)) == -1) {
    perror("setsockopt(TCP_KEEPIDLE)");
  }

  if ((nbytes = recv(fd, buf, MAX_SIZE - 1, 0)) == -1) {
    perror("recv");
    return 3;
  }

  buf[nbytes] = '\0';

  printf("%s", buf);

  while (is_true) {
    is_true = some_func(fd);
  }

  close(fd);

  return 0;
}
