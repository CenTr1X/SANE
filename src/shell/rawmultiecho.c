#include "rawmultiecho.h"
#include "lwip/sockets.h"
#include "lan91c.h"
#include "TcpIp.h"

static struct netif *netif = &(gLan91c.netif[0]);


// 设置组播地址
#define MULTICAST_IP_ADDR "239.255.0.1"
// 设置组播端口
#define MULTICAST_PORT 5000

void raw_multiecho_client()
{
    // 创建UDP套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("sock:%d", sockfd);
    if (sockfd < 0) {
        perror("socket creation failed");
        return;
    }
    // 设置套接字选项，允许发送组播数据包
    int enable = 1;
    /*if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &enable, sizeof(enable)) < 0) {
        perror("setsockopt IP_MULTICAST_IF failed");
        close(sockfd);
        return;
    }*/

    // 设置组播目的地的IP地址和端口
    struct sockaddr_in multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP_ADDR);
    multicast_addr.sin_port = htons(MULTICAST_PORT);
    //multicast_addr.sin_addr.s_addr = inet_addr("10.0.0.5");
    // 发送数据
    const char* data = "Hello, multicast!";
    if (sendto(sockfd, data, strlen(data), 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
        perror("sendto failed");
        close(sockfd);
        return;
    }

    // 关闭套接字
    close(sockfd);
}

void raw_multiecho_server()
{
    // 创建UDP套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("sock:%d", sockfd);
    if (sockfd < 0) {
        perror("socket creation failed");
        return;
    }
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse, sizeof(reuse));
    // 设置套接字选项，允许接收组播数据包
    int enable = 1;
    /*if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &enable, sizeof(enable)) < 0) {
        perror("setsockopt IP_MULTICAST_IF failed");
        close(sockfd);
        return;
    }*/

    // 绑定本地IP地址和端口
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    //local_addr.sin_addr.s_addr = netif->ip_addr.addr;
    local_addr.sin_port = htons(MULTICAST_PORT);
    if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return;
    }

    // 加入组播组
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP_ADDR);
    mreq.imr_interface.s_addr = netif->ip_addr.addr;
    
    //mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
        perror("setsockopt IP_ADD_MEMBERSHIP failed");
        close(sockfd);
        return;
    }

    // 接收数据
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&sender_addr, &sender_addr_len);
    if (recv_len < 0) {
        perror("recvfrom failed");
        close(sockfd);
        return;
    }

    // 打印接收到的数据
    printf("Received multicast message: %s\n", buffer);

    // 关闭套接字
    close(sockfd);
}