#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>


#define HOST_ADDR "www.example.net"

int main() {
  int status;
  struct addrinfo hints;
  struct addrinfo* res;
  struct addrinfo* p;
  char ipstr[INET6_ADDRSTRLEN];
  char my_name[200];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  //hints.ai_flags = AI_PASSIVE;

  if((status = getaddrinfo(HOST_ADDR, NULL, &hints, &res) != 0)) {
    fprintf(stderr, "lala %s\n", gai_strerror(status));
    return 2;
  }
  printf("IP for %s \n", HOST_ADDR);

  for(p = res; p != NULL; p = p->ai_next) {
    void* addr;
    char* ipver;
    if(p->ai_family == AF_INET) {
      struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    } else {
      struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    printf(" %s: %s\n", ipver, ipstr);
  }
  gethostname(my_name, 200);
  printf("%s\n", my_name);
  freeaddrinfo(res);
  return 0;
}