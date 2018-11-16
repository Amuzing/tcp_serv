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
#define MAX_NAME 64
#define MAX_SIZE 256

#define CMD_NAME "0 "
#define CMD_SAVE "1 "
#define CMD_REM "2 "
#define CMD_LIST "3 "

int send_name(int fd, char* buf);

bool handle_command(int fd, char* buf);

int do_string(int fd, char* cmd, char* buf);
int read_strings(int fd, char* buf);

int send_msg(int fd, char* msg);

int main(int argc, char* argv[]) {
  int fd;
  struct addrinfo hints, *ai, *p;
  int nbytes;
  int yes = 1;
  int idle_time = 60;
  int rv;
  bool is_true = true;;
  char* address = NULL;

  char name[MAX_NAME];
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
  if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &idle_time, sizeof(idle_time)) ==
      -1) {
    perror("setsockopt(TCP_KEEPIDLE)");
  }

  if ((nbytes = recv(fd, buf, MAX_SIZE - 1, 0)) == -1) {
    perror("recv");
    return 3;
  }

  buf[nbytes] = '\0';

  printf("%s\n", buf);

  printf("enter your name:\n");
  scanf("%s", name);
  do_string(fd, CMD_NAME, name);

  while (is_true) {
    is_true = handle_command(fd, buf);
  }

  close(fd);

  return 0;
}

int send_msg(int fd, char* buf) {
  if (send(fd, buf, sizeof(buf), 0) == -1) {
    perror("failed to send");
    return 1;
  }
  return 0;
}

int do_string(int fd, char* cmd, char* buf) {
  char temp[MAX_SIZE + sizeof(cmd)];
  strcpy(temp, cmd);
  strcat(temp, buf);
  return send_msg(fd, temp);
}

int read_strings(int fd, char* buf) {
  int nbytes;
  bool is_true = true;
  do_string(fd, CMD_LIST, buf);
  while (is_true) {
    if ((nbytes = recv(fd, buf, sizeof(buf), 0)) <= 0) {
      if (nbytes == 0) {
        printf("connection on socket %d was closed\n", fd);
        return 1;
      } else {
        perror("failed to recv");
        return 2;
      }
    } else {
      if(strlen(buf) != strlen("$") || strcmp(buf, "$")) {
        printf("%s\n", buf);
      } else {
        is_true = false;
      }
    }
  }
  return 0;
}

bool handle_command(int fd, char* buf) {
  char c;
  int cmd;
  int rv = 0;;
  printf("enter one of the commands, listed above, \"0\" to finish the work: \n");
  while ((c = getchar()) != '\n' && c != EOF) {
    cmd = c - '0';
  }
  switch(cmd) {
    case 1: {
      printf("enter the message to save:\n");
      scanf("%s", buf);
      rv = do_string(fd, CMD_SAVE, buf);
      break;
    }
    case 2: {
      printf("enter the message to remove:\n");
      scanf("%s", buf);
      rv = do_string(fd, CMD_REM, buf);
      break;
    }
    case 3: {
      rv = read_strings(fd, buf);
      break;
    }
    case 0: {
      printf("finishing...\n");
      rv = 3;
    }
    default : {
      printf("unrecognized command %d\n", c);
    }
  }
  return !rv;
}


