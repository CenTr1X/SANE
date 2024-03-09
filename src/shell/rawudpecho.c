#include "rawudpecho.h"

#include "lwip/sockets.h"
#include "FreeRTOS.h"

void raw_udpecho_client() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[10] = "test1234";

    // 创建 UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Failed to create socket\n");
        return;
    }
    int on = 1;
    //ioctl(sockfd, FIONBIO, &on);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);

    // 将服务器 IP 地址转换为网络字节序
    if (inet_aton("10.0.0.5", &server_addr.sin_addr) == 0) {
        printf("Invalid server IP address\n");
        close(sockfd);
        return;
    }



        // 发送数据到服务器
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
            printf("Failed to send\n");
        }

        // 接收服务器返回的数据
        struct sockaddr_in remote_addr;
        socklen_t remote_len = sizeof(remote_addr);
        printf("before recvfrom\n");
        int recv_len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&remote_addr, &remote_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Received: %s\n", buffer);
        }

    close(sockfd);
}

void raw_udpecho_server() {

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[10];

    // 创建 UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Failed to create socket\n");
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定服务器地址和端口
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("Failed to bind\n");
        close(sockfd);
        return;
}
    printf("UDP Echo Server started on port %d\n", 12345);


        client_len = sizeof(client_addr);

        // 接收数据
        int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Received: %s", buffer);

            // 发送数据回客户端
            if (sendto(sockfd, buffer, recv_len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
                printf("Failed to send\n");
            }
        }

    close(sockfd);
}