#include "tcp_comm_conn.h"

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

void serialize_uint(uint32_t value, uint8_t* buf) 
{
  /*buf[0] =  value        & HEX_MASK;
  buf[1] = (value >> 8)  & HEX_MASK;
  buf[2] = (value >> 16) & HEX_MASK;
  buf[3] = (value >> 24) & HEX_MASK;*/
  value = htonl(value);
  memcpy(buf, (void*)&value, sizeof(value));
}

uint32_t deserialize_uint(uint8_t* buf) 
{
  /*unsigned int value = (buf[3] << 24) + 
                       (buf[2] << 16) + 
                       (buf[1] << 8) + 
                        buf[0];*/
  uint32_t temp_val = 0;
  memcpy(&temp_val, buf, sizeof(temp_val));
  return ntohl(temp_val);
}

int send_string(int fd, const char* buf, size_t size, int flags) 
{
  uint32_t total_size = MSG_SIZE_LEN + size;
  char* temp = malloc(total_size); 
  if (temp == NULL) {
    fprintf(stderr, "An error occured while allocating memory. \n");
    return -3;
  }
  strncpy(temp + MSG_SIZE_LEN, buf, size);
  serialize_uint(total_size, (uint8_t*)temp);
  ssize_t total_bytes_sent = 0;
  ssize_t bytes_left = total_size;
  ssize_t n = 0;
  printf("Sending %s %u bytes long...\n", buf, total_size);
  while(total_bytes_sent < total_size) {
    n = send(fd, temp + total_bytes_sent, bytes_left, flags);
    printf("We sent %ld bytes...\n", n);
    if (n == -1) { 
      perror("send: ");
      break; 
    }
    total_bytes_sent += n;
    bytes_left -= n;
  }
  free(temp);
  return (total_size == total_bytes_sent) ? 0 : -2;
}

int recv_string(int fd, char** buf, uint32_t* len, int flags) 
{
  uint8_t msg_len[MSG_SIZE_LEN];
  ssize_t total_bytes_received = 0;
  ssize_t bytes_left = sizeof(msg_len);
  ssize_t n = 0;
  if ((n = recv(fd, msg_len, sizeof(msg_len), 0)) != sizeof(msg_len)) {
    if (n == -1) {
      perror("recv");
      return -1;
    } else {
      //TODO
      printf(":(\n");
    }
  }
  *len = deserialize_uint(msg_len) - MSG_SIZE_LEN;
  *buf = malloc(sizeof(char) * (*len) + 1);
  if (buf == NULL) {
    fprintf(stderr, "An error occured while allocating memory. \n");
    return -3;
  }
  total_bytes_received = 0;
  bytes_left = *len;
  while (total_bytes_received < bytes_left) {
    n = recv(fd, *buf + total_bytes_received, bytes_left, 0);
    printf("We received %ld bytes...\n", n);
    if (n == -1) { 
      perror("recv: ");
      break; 
    }
    total_bytes_received += n;
    bytes_left -= n;
  }
  buf[total_bytes_received] = '\0';
  printf("Received %s %u bytes long...\n", *buf, *len);
  return (*len == total_bytes_received) ? 0 : -2;
}
