#include "TcpIp.h"

#include <lwip/sockets.h>
#include <string.h>
#include <lwip/igmp.h>

#include "lan91c.h"
#include "printf.h"
/* ================================ [ DATAS     ] ============================================== */
static struct netif *netif = &(gLan91c.netif[0]);

/* ================================ [ FUNCTIONS ] ============================================== */
TcpIp_SocketIdType TcpIp_Create(TcpIp_ProtocolType protocol) {
  TcpIp_SocketIdType sockId;
  int type;
  int on = 1;

  if (TCPIP_IPPROTO_TCP == protocol) {
    type = SOCK_STREAM;
  } else {
    type = SOCK_DGRAM;
  }

  sockId = socket(AF_INET, type, 0);
  if (sockId >= 0) {
    setsockopt(sockId, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    if (TCPIP_IPPROTO_UDP == protocol) {
      TcpIp_SetNonBlock(sockId, TRUE);
    }
  }

  return sockId;
}

Std_ReturnType TcpIp_Close(TcpIp_SocketIdType SocketId, boolean Abort) {
  int r;
  Std_ReturnType ret = E_OK;
  (void)Abort;
  r = close(SocketId);
  if (0 != r) {
    ret = E_NOT_OK;
  }

  return ret;
}

Std_ReturnType TcpIp_Bind(TcpIp_SocketIdType SocketId, TcpIp_LocalAddrIdType LocalAddrId,
                          uint16_t *PortPtr) {
  Std_ReturnType ret = E_OK;
  int r;
  int on = 1;
  struct sockaddr_in sLocalAddr;
  //set option of this socket
  setsockopt(SocketId, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int)); /* Set socket to no delay */
  
  memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
  sLocalAddr.sin_family = AF_INET;
  sLocalAddr.sin_len = sizeof(sLocalAddr);
  sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sLocalAddr.sin_port = htons(*PortPtr);
  
  r = bind(SocketId, (struct sockaddr *)&sLocalAddr, sizeof(sLocalAddr));
  if (0 != r) {
    ret = E_NOT_OK;
  } else {
    TcpIp_SetNonBlock(SocketId, TRUE);
  }

  return ret;
}

Std_ReturnType TcpIp_TcpListen(TcpIp_SocketIdType SocketId, uint16_t MaxChannels) {
  Std_ReturnType ret = E_NOT_OK;
  int r;

  r = listen(SocketId, MaxChannels);
  if (0 == r) {
    ret = E_OK;
  } else {
    printf("[%d] listen failed: %d\n", SocketId, r);
  }
  printf("[%d] listen(%d) \n", SocketId, MaxChannels);

  return ret;
}

Std_ReturnType TcpIp_TcpConnect(TcpIp_SocketIdType SocketId,
                                const TcpIp_SockAddrType *RemoteAddrPtr) {
  Std_ReturnType ret = E_NOT_OK;
  int r;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  memcpy(&addr.sin_addr.s_addr, RemoteAddrPtr->addr, 4);
  addr.sin_port = htons(RemoteAddrPtr->port);

  r = connect(SocketId, (struct sockaddr *)&addr, sizeof(struct sockaddr));
  if (0 == r) {
    TcpIp_SetNonBlock(SocketId, TRUE);
    ret = E_OK;
    printf(
      "[%d] connect %d.%d.%d.%d:%d\n", SocketId, RemoteAddrPtr->addr[0], RemoteAddrPtr->addr[1],
      RemoteAddrPtr->addr[2], RemoteAddrPtr->addr[3], RemoteAddrPtr->port);
  } else {
    printf("connect fail: %s\n", strerror(errno));
    printf("[%d] connect %d.%d.%d.%d:%d failed: %d\n", SocketId, RemoteAddrPtr->addr[0],
                   RemoteAddrPtr->addr[1], RemoteAddrPtr->addr[2], RemoteAddrPtr->addr[3],
                   RemoteAddrPtr->port, r);
  }



  return ret;
}

Std_ReturnType TcpIp_AddToMulticast(TcpIp_SocketIdType SocketId, TcpIp_SockAddrType *ipv4Addr) {
  int r;
  Std_ReturnType ret = E_NOT_OK;

  ip_addr_t ipaddr;
  struct ip_mreq mreq;

  int enable = 1;

  memcpy(&ipaddr.addr, ipv4Addr->addr, 4);
  if (ip_addr_ismulticast(&ipaddr)) {
    printf("ip_addr_ismulticast(&ipaddr)\n");
    mreq.imr_multiaddr.s_addr = ipaddr.addr;
    mreq.imr_interface.s_addr = netif->ip_addr.addr;

    r = setsockopt(SocketId, IPPROTO_IP, IP_MULTICAST_IF, &enable, sizeof(enable));
    printf("r==0?:%d\n", r==0);
    r = setsockopt(SocketId, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    printf("r==0?:%d\n", r==0);
    //printf("err:%d, %s",errno,  strerror(errno));
    if (0 == r) {
      printf("0 == r\n");
      ret = E_OK;
    }
  }
  return ret;
}

Std_ReturnType TcpIp_TcpAccept(TcpIp_SocketIdType SocketId, TcpIp_SocketIdType *AcceptSock,
                               TcpIp_SockAddrType *RemoteAddrPtr) {
  Std_ReturnType ret = E_OK;
  int clientFd;
  struct sockaddr_in client_addr;
  int addrlen = sizeof(client_addr);
  //printf("ready to accept\n");
  clientFd = accept(SocketId, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
  //printf("Accepted! clientfd:%d\n", clientFd);
  if (clientFd >= 0) {
    /* New connection established */
    TcpIp_SetNonBlock(clientFd, TRUE);

    TcpIp_TcpKeepAlive(clientFd, 10, 1, 3);
    RemoteAddrPtr->port = htons(client_addr.sin_port);
    memcpy(RemoteAddrPtr->addr, &client_addr.sin_addr.s_addr, 4);
    printf("[%d] accept %d.%d.%d.%d:%d\n", SocketId, RemoteAddrPtr->addr[0], RemoteAddrPtr->addr[1],
           RemoteAddrPtr->addr[2], RemoteAddrPtr->addr[3], RemoteAddrPtr->port);
    *AcceptSock = (TcpIp_SocketIdType)clientFd;
  } else {
    ret = E_NOT_OK;
  }

  return ret;
}

//can only used in tcp, not udp
Std_ReturnType TcpIp_Recv(TcpIp_SocketIdType SocketId, uint8_t *BufPtr,
                          uint32_t *Length /* InOut */) {
  Std_ReturnType ret = E_OK;
  int nbytes;

  nbytes = recv(SocketId, (char *)BufPtr, *Length, 0);

  *Length = 0;
  if (nbytes > 0) {
    *Length = nbytes;
    printf("[%d] recv %d bytes\n", SocketId, nbytes);
  } else if (nbytes < -1) {
    ret = nbytes;
    printf("[%d] recv got error %d\n", SocketId, nbytes);
  }
  return ret;
}

Std_ReturnType TcpIp_RecvFrom(TcpIp_SocketIdType SocketId, TcpIp_SockAddrType *RemoteAddrPtr,
                              uint8_t *BufPtr, uint32_t *Length /* InOut */) {
  Std_ReturnType ret = E_OK;
  struct sockaddr_in fromAddr;
  socklen_t fromAddrLen = sizeof(fromAddr);
  int nbytes;

  nbytes =
    recvfrom(SocketId, (char *)BufPtr, *Length, 0, (struct sockaddr *)&fromAddr, &fromAddrLen);
  *Length = 0;
  if (nbytes > 0) {
    RemoteAddrPtr->port = htons(fromAddr.sin_port);
    memcpy(RemoteAddrPtr->addr, &fromAddr.sin_addr.s_addr, 4);
    // printf("[%d] recv %d bytes from %d.%d.%d.%d:%d\n", SocketId, nbytes,
    //               RemoteAddrPtr->addr[0], RemoteAddrPtr->addr[1], RemoteAddrPtr->addr[2],
    //               RemoteAddrPtr->addr[3], RemoteAddrPtr->port);
    *Length = nbytes;
  } else if (nbytes < -1) {
    ret = nbytes;
    printf("[%d] recvfrom got error %d\n", SocketId, nbytes);
  } else {
    /* got nothing */
  }

  return ret;
}

//set socket non-blocked
Std_ReturnType TcpIp_SetNonBlock(TcpIp_SocketIdType SocketId, boolean nonBlocked) {
  Std_ReturnType ret = E_OK;
  int r;
  int on = (int)nonBlocked;
  r = ioctl(SocketId, FIONBIO, &on);

  if (0 != r) {
    ret = E_NOT_OK;
  }
  //printf("set nonblock for %d", SocketId);
  return ret;
}

Std_ReturnType TcpIp_TcpKeepAlive(TcpIp_SocketIdType SocketId, uint32_t Idel, uint32_t Interval,
                                  uint32_t Count) {
  Std_ReturnType ret = E_OK;
  int keepalive = 1;


  int r = 0;
  r |= setsockopt(SocketId, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));

  if (0 != r) {
    ret = E_NOT_OK;
    printf("[%d] keep alive fail\n", SocketId);
  }

  return ret;
}

/* Get the number of bytes that are immediately available for	reading.*/
uint16_t TcpIp_Tell(TcpIp_SocketIdType SocketId) {
  int Length = 0;
  //printf("tell socket id of %d", SocketId);
  ioctl(SocketId, FIONREAD, &Length);

  if (Length > TCPIP_MAX_DATA_SIZE) {
    Length = TCPIP_MAX_DATA_SIZE;
  }

  return Length;
}

Std_ReturnType TcpIp_IsTcpStatusOK(TcpIp_SocketIdType SocketId) {
  Std_ReturnType ret = E_OK;
  int sockErr = 0, r;
  socklen_t sockErrLen = sizeof(sockErr);

  r = getsockopt(SocketId, SOL_SOCKET, SO_ERROR, (char *)&sockErr, &sockErrLen);
  if (r != 0) {
    ret = E_NOT_OK;
  } else {
    if ((sockErr != 0) && (sockErr != EWOULDBLOCK)) {
      ret = E_NOT_OK;
      printf("[%d] status bad\n", SocketId);
    }
  }

  return ret;
}

Std_ReturnType TcpIp_SetupAddrFrom(TcpIp_SockAddrType *RemoteAddrPtr, uint32_t ipv4Addr,
                                   uint16_t port) {
  uint32_t u32Addr = htonl(ipv4Addr);
  memcpy(RemoteAddrPtr->addr, &u32Addr, 4);
  RemoteAddrPtr->port = port;
  return E_OK;
}

Std_ReturnType TcpIp_SendTo(TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType *RemoteAddrPtr,
                            const uint8_t *BufPtr, uint32_t Length) {
  Std_ReturnType ret = E_OK;
  struct sockaddr_in toAddr;
  socklen_t toAddrLen = sizeof(toAddr);
  int nbytes;

  toAddr.sin_family = AF_INET;
  toAddr.sin_len = sizeof(toAddr);

  memcpy(&toAddr.sin_addr.s_addr, RemoteAddrPtr->addr, 4);
  toAddr.sin_port = htons(RemoteAddrPtr->port);
  nbytes = sendto(SocketId, (char *)BufPtr, Length, 0, (struct sockaddr *)&toAddr, toAddrLen);

  // printf("[%d] send to %d.%d.%d.%d:%d %d/%d bytes\n", SocketId, RemoteAddrPtr->addr[0],
  //               RemoteAddrPtr->addr[1], RemoteAddrPtr->addr[2], RemoteAddrPtr->addr[3],
  //               RemoteAddrPtr->port, nbytes, Length);

  if (nbytes != Length) {
    // printf("[%d] sendto(%d.%d.%d.%d:%d, %d), error is %d\n", SocketId,
    //                RemoteAddrPtr->addr[0], RemoteAddrPtr->addr[1], RemoteAddrPtr->addr[2],
    //                RemoteAddrPtr->addr[3], RemoteAddrPtr->port, Length, nbytes);
    if (nbytes >= 0) {
      ret = TCPIP_E_NOSPACE;
    } else {
      ret = E_NOT_OK;
    }
  }

  return ret;
}

Std_ReturnType TcpIp_Send(TcpIp_SocketIdType SocketId, const uint8_t *BufPtr, uint32_t Length) {
  Std_ReturnType ret = E_OK;
  int nbytes;

  nbytes = send(SocketId, (char *)BufPtr, Length, 0);
  printf("[%d] send(%d/%d)\n", SocketId, nbytes, Length);

  if (nbytes != Length) {
    printf("[%d] send(%d), error is %d\n", SocketId, Length, nbytes);
    if (nbytes >= 0) {
      ret = TCPIP_E_NOSPACE;
    } else {
      ret = E_NOT_OK;
    }
  }

  return ret;
}

Std_ReturnType TcpIp_GetLocalAddr(TcpIp_SocketIdType SocketId, TcpIp_SockAddrType *addr) {
  Std_ReturnType ret = E_NOT_OK;
  int r;
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);

  r = getsockname(SocketId, (struct sockaddr *)&name, &namelen);
  if (0 == r) {
    addr->port = htons(name.sin_port);
    memcpy(addr->addr, &name.sin_addr, 4);
    ret = E_OK;

    // printf("[%d] sockname %d.%d.%d.%d:%d\n", SocketId, addr->addr[0], addr->addr[1],
    //               addr->addr[2], addr->addr[3], addr->port);
  }

  return ret;
}

Std_ReturnType TcpIp_GetIpAddr(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_SockAddrType *IpAddrPtr,
                               uint8 *NetmaskPtr, TcpIp_SockAddrType *DefaultRouterPtr) {

  memcpy(IpAddrPtr->addr, &netif->ip_addr.addr, 4);
  // printf("sockname %d.%d.%d.%d\n", IpAddrPtr->addr[0], IpAddrPtr->addr[1],
  //                 IpAddrPtr->addr[2], IpAddrPtr->addr[3]);
  return E_OK;
}

Std_ReturnType TcpIp_OpenNetIfIgmp()
{
  printf("TcpIp_OpenNetIfIgmp");
  netif->flags |= NETIF_FLAG_IGMP;
  igmp_start(netif);
}

uint32_t TcpIp_InetAddr(const char *ip) {
  uint32_t u32Addr;
  u32Addr = inet_addr(ip);
  u32Addr = ntohl(u32Addr);
  return u32Addr;
}
