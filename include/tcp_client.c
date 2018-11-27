#include "tcp_client.h"

int get_conn_socket(const char *address, const char *port) {
  struct addrinfo *ai, *p;

  int rv = 0;
  if ((rv = get_addr_info(address, port, &ai)) != 0) {
    return rv;
  }

  int connected_fd = -1;
  for (p = ai; p != NULL; p = p->ai_next) {
    if ((connected_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
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
    return -201;
  }

  freeaddrinfo(ai);

  return connected_fd;
}

int send_msg(int fd, char *buf) {
  printf("sending:  %s\n", buf);
  if (send_string(fd, buf, strlen(buf), 0) == -1) {
    perror("failed to send");
    return 1;
  }
  return 0;
}

int send_cmd_to_server(int fd, char *cmd, char *buf, size_t max_size) {
  size_t cmd_size = strlen(cmd);
  size_t buf_len = strlen(buf);
  size_t temp_len = cmd_size + buf_len + 1;  // 1 - '\0'
  size_t len = (temp_len > max_size) ? max_size : temp_len;
  printf(" cmd %lu  buf %lu  temp %lu ", cmd_size, buf_len, temp_len);
  char temp[len];
  memset(temp, 0, sizeof(temp));
  strncpy(temp, cmd, cmd_size);
  strncat(temp, buf, len - cmd_size - 1);  // 1 - '\0'
  return send_msg(fd, temp);
}

int send_name(int fd, char *name, size_t max_size) {
  return send_cmd_to_server(fd, CMD_NAME, name, max_size);
}

int read_strings(int fd, char *buf, size_t max_size) {
  char *msg = NULL;
  uint32_t len = 0;

  send_cmd_to_server(fd, CMD_LIST, buf, max_size);

  if (recv_string(fd, msg, &len, 0) == 0) {
    printf("%s\n", msg);

    free(msg);
  }

  return 0;
}

bool handle_command(int fd, char *buf, size_t max_size) {
  char cmd = '0';
  int rv = 0;
  char temp[256];
  char line[256];
  printf(
      "enter one of the commands, listed above, \"0\" to finish the work:\n");
  fflush(stdout);
  scanf(" %c", &cmd);
  // cmd = temp[0];
  switch (cmd) {
    case '0': {
      printf("finishing...\n");
      rv = 3;
      break;
    }
    case '1': {
      printf("enter the message to save:\n");
      // line = malloc(max_size);
      if (line != NULL) {
        // fflush(stdin);
        fgets(line, max_size, stdin);
        scanf("%s", line);
        rv = send_cmd_to_server(fd, CMD_SAVE, line, max_size);
        // free(line);
        // line = NULL;
      } else {
        printf("no memory to allocate :(\n");
      }
      // scanf("%256[0-9a-zA-Z]", buf);
      break;
    }
    case '2': {
      printf("enter the message to remove: ");
      // line = malloc(max_size);
      if (line != NULL) {
        fgets(line, max_size, stdin);
        rv = send_cmd_to_server(fd, CMD_REM, line, max_size);
        //(line);
        // line = NULL;
      } else {
      }
      break;
    }
    case '3': {
      rv = read_strings(fd, buf, max_size);
      break;
    }
    default: { printf("unrecognized command %c\n", cmd); }
  }
  return !rv;
}
