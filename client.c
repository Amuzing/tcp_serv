#include "tcp_client.h"


int main(int argc, char* argv[]) {
  
  bool is_true = true;

  char name[MAX_NAME];
  char buf[MAX_SIZE];

  char* address = (argc == 2) ? argv[1] :  NULL;
  int fd = -1;
  if((fd = get_conn_socket(address, PORT)) < 0) {
    //TODO
    printf(":(\n");
    return 1;
  }

  // keep-alive
  int keep_idle = 60;
  set_sock_keepalive_opt(fd, &keep_idle, NULL, NULL);

  /*if ((nbytes = recv(fd, buf, MAX_SIZE - 1, 0)) == -1) {
    perror("recv");
    return 3;
  }*/

  char* msg = NULL;
  uint32_t len;
  recv_string(fd, &msg, &len, 0);

  //buf[nbytes] = '\0';

  printf("%s\n", msg);
  free(msg);
  msg = NULL;

  printf("enter your name:\n");
  scanf("%s", name);
  send_name(fd, name, MAX_SIZE);

  while (is_true) {
    is_true = handle_command(fd, buf, sizeof(buf));
  }

  close(fd);
  fd = -1;

  return 0;
}
