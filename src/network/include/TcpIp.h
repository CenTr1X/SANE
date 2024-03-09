#ifndef _TCPIP_H
#define _TCPIP_H
/* ================================ [ INCLUDES  ] ============================================== */
#include "Std_Types.h"
/* ================================ [ MACROS    ] ============================================== */
/* @SWS_TCPIP_00135 */
#define TCPIP_LOCALADDRID_ANY ((TcpIp_LocalAddrIdType)0xFF)

#define TCPIP_E_NOSPACE ((Std_ReturnType)0x10)

#ifndef TCPIP_MAX_DATA_SIZE
#define TCPIP_MAX_DATA_SIZE 1420
#endif

#define TCPIP_IPV4_ADDR(b0, b1, b2, b3)                                                            \
  ((((uint32_t)b0) << 24) + (((uint32_t)b1) << 16) + (((uint32_t)b2) << 8) + b3)

#define TCPIP_AF_INET ((TcpIp_DomainType)0x02)
#define TCPIP_AF_INET6 ((TcpIp_DomainType)0x1c)
/* ================================ [ TYPES     ] ============================================== */
typedef int TcpIp_SocketIdType;

/* @SWS_TCPIP_00009 */
typedef uint16_t TcpIp_DomainType;

/* @SWS_TCPIP_00030 */
typedef uint8_t TcpIp_LocalAddrIdType;

/* @SWS_TCPIP_00010 */
typedef enum
{
  TCPIP_IPPROTO_TCP = 0x06,
  TCPIP_IPPROTO_UDP = 0x11,
} TcpIp_ProtocolType;

/* @SWS_TCPIP_00012 */
typedef struct {
  TcpIp_DomainType domain;
  uint16_t port;
  uint8_t addr[4]; /* NOTE: now only support IPv4 */
} TcpIp_SockAddrType;

/* ================================ [ DECLARES  ] ============================================== */
TcpIp_SocketIdType TcpIp_Create(TcpIp_ProtocolType protocol);

/* @SWS_TCPIP_00017 */
Std_ReturnType TcpIp_Close(TcpIp_SocketIdType SocketId, boolean Abort);

/* @SWS_TCPIP_00015 */
Std_ReturnType TcpIp_Bind(TcpIp_SocketIdType SocketId, TcpIp_LocalAddrIdType LocalAddrId,
                          uint16_t *PortPtr);

/* @SWS_TCPIP_00023 */
Std_ReturnType TcpIp_TcpListen(TcpIp_SocketIdType SocketId, uint16_t MaxChannels);

/* @SWS_TCPIP_00022 */
Std_ReturnType TcpIp_TcpConnect(TcpIp_SocketIdType SocketId,
                                const TcpIp_SockAddrType *RemoteAddrPtr);

Std_ReturnType TcpIp_AddToMulticast(TcpIp_SocketIdType SocketId, TcpIp_SockAddrType *ipv4Addr);

Std_ReturnType TcpIp_SetNonBlock(TcpIp_SocketIdType SocketId, boolean nonBlocked);

Std_ReturnType TcpIp_TcpAccept(TcpIp_SocketIdType SocketId, TcpIp_SocketIdType *AcceptSock,
                               TcpIp_SockAddrType *RemoteAddrPtr);

Std_ReturnType TcpIp_Recv(TcpIp_SocketIdType SocketId, uint8_t *BufPtr,
                          uint32_t *Length /* InOut */);

Std_ReturnType TcpIp_RecvFrom(TcpIp_SocketIdType SocketId, TcpIp_SockAddrType *RemoteAddrPtr,
                              uint8_t *BufPtr, uint32_t *Length /* InOut */);

Std_ReturnType TcpIp_SendTo(TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType *RemoteAddrPtr,
                            const uint8_t *BufPtr, uint32_t Length);

Std_ReturnType TcpIp_Send(TcpIp_SocketIdType SocketId, const uint8_t *BufPtr, uint32_t Length);

/*
 * Idel: The time (in seconds) the connection needs to remain idle before TCP starts sending
 * keepalive probes,
 * Interval: The time (in seconds) between individual keepalive probes.
 * Count: The maximum number of keepalive probes TCP should send before dropping the connection.
 **/
Std_ReturnType TcpIp_TcpKeepAlive(TcpIp_SocketIdType SocketId, uint32_t Idel, uint32_t Interval,
                                  uint32_t Count);

//get length?
uint16_t TcpIp_Tell(TcpIp_SocketIdType SocketId);

Std_ReturnType TcpIp_IsTcpStatusOK(TcpIp_SocketIdType SocketId);

Std_ReturnType TcpIp_SetupAddrFrom(TcpIp_SockAddrType *RemoteAddrPtr, uint32_t ipv4Addr,
                                   uint16_t port);

Std_ReturnType TcpIp_GetLocalAddr(TcpIp_SocketIdType SocketId, TcpIp_SockAddrType *addr);

Std_ReturnType TcpIp_GetIpAddr(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_SockAddrType *IpAddrPtr,
                               uint8 *NetmaskPtr, TcpIp_SockAddrType *DefaultRouterPtr);

Std_ReturnType TcpIp_OpenNetIfIgmp();

uint32_t TcpIp_InetAddr(const char *ip);
#endif