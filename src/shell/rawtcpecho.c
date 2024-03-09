#include "rawtcpecho.h"

#include "lwip/sockets.h"
#include "FreeRTOS.h"

void raw_tcpecho_client() {
  struct sockaddr_in server_addr;
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  char *buf = "test";
  printf("sock fd:%d\n", sock_fd);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(30490);
  server_addr.sin_len = sizeof("10.0.0.5");
  inet_pton(AF_INET, "10.0.0.5", &server_addr.sin_addr);
  printf("going to connect!\n");
  int re = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  printf("connected! going to send!, re:%d\n", re);
  send(sock_fd, buf, strlen(buf), 0);
  printf("sent!\n");
}

void raw_tcpecho_server() {
  int sock_listener = socket(AF_INET, SOCK_STREAM, 0);
  printf("sock listener:%d\n", sock_listener);
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(30490);

  inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);  // 255.255.255.255?
  bind(sock_listener, (struct sockaddr *)&server_addr, sizeof(server_addr));
  listen(sock_listener, 10);

  struct sockaddr_in client_addr;
  socklen_t client_addr_size = sizeof(client_addr);
  int sock_client;
  printf("going to accept!\n");
  sock_client =
      accept(sock_listener, (struct sockaddr *)&client_addr, &client_addr_size);
    printf("sock client:%d\n", sock_client);
    printf("accepted! going to recv\n");
  char msg_buf[4];
  int bytes;
  int length = 0;
  vTaskDelay(pdMS_TO_TICKS(3000));
  ioctl(sock_client, FIONREAD, &length);
  printf("length1: %d", length);
  ioctl(sock_listener, FIONREAD, &length);
  printf("length2: %d", length);
  bytes = recv(sock_client, &msg_buf, 4, 0);
  printf("get msg: %s", msg_buf);
}