/* ================================ [ INCLUDES  ]
 * ============================================== */

#include "SoAd.h"
#include "printf.h"
#include "print.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "SoAd_Cfg.h"
#include "debug.h"

/* ================================ [ DECLARES  ]
 * ============================================== */
extern const SoAd_ConfigType SoAd_Config;
/* ================================ [ MACROS    ]
 * ============================================== */
#define SOAD_CONFIG (&SoAd_Config)
#define IS_CON_TYPE_OF(con, mask) (0 != ((con)->SoConType & (mask)))

/* ================================ [ FUNCTIONS ]
 * ============================================== */
static void soAdSocketIfRxNotify(SoAd_SocketContextType *context,
                                 const SoAd_SocketConnectionType *connection,
                                 uint8_t *data, uint16_t rxLen) {
  const SoAd_SocketConnectionGroupType *conG =
      &SOAD_CONFIG->ConnectionGroups[connection->GID];
  const SoAd_IfInterfaceType *IF =
      (const SoAd_IfInterfaceType *)conG->Interface;
  PduInfoType PduInfo;
  SANE_DEBUGF(SANE_DBG_TRACE,("connection->GID: %d\n", connection->GID));
  PduInfo.SduDataPtr = data;
  PduInfo.SduLength = rxLen;
  PduInfo.MetaDataPtr = (uint8_t *)&context->RemoteAddr;
  if (IF->IfRxIndication) {
    IF->IfRxIndication(connection->RxPduId, &PduInfo);
  }
}

static void soAdSocketTpRxNotify(SoAd_SocketContextType *context,
                                 const SoAd_SocketConnectionType *connection,
                                 uint8_t *data, uint16_t rxLen) {
  const SoAd_SocketConnectionGroupType *conG =
      &SOAD_CONFIG->ConnectionGroups[connection->GID];
  const SoAd_TpInterfaceType *IF =
      (const SoAd_TpInterfaceType *)conG->Interface;
  PduInfoType PduInfo;
  PduInfo.SduDataPtr = data;
  PduInfo.SduLength = rxLen;
  PduInfo.MetaDataPtr = (uint8_t *)&context->RemoteAddr;
  PduLengthType bufferSize;

  if (IF->TpCopyRxData) {
    IF->TpCopyRxData(connection->RxPduId, &PduInfo, &bufferSize);
  }
}

void soAdCreateSocket(int index) {
  const SoAd_SocketConnectionType *connection =
      &SOAD_CONFIG->Connections[index];
  const SoAd_SocketConnectionGroupType *conG =
      &SOAD_CONFIG->ConnectionGroups[connection->GID];
  SoAd_SocketContextType *context = &SOAD_CONFIG->Contexts[index];
  TcpIp_SocketIdType sockId;
  Std_ReturnType ret = E_OK;

  // create socket
  sockId = TcpIp_Create(conG->ProtocolType);
  //printf("sockid:%d\n", sockId);
  // check socket and bind
  if (sockId >= 0) {
    // except TCP client
    if (IS_CON_TYPE_OF(connection, SOAD_SOCON_TCP_SERVER |
                                       SOAD_SOCON_UDP_SERVER |
                                       SOAD_SOCON_UDP_CLIENT)) {
      ret = TcpIp_Bind(sockId, conG->LocalAddrId, &conG->Port);
      // handle multicast
      if ((E_OK == ret) && conG->IsMulitcast) {
        ret = TcpIp_AddToMulticast(sockId, &context->RemoteAddr);
      }
      // handle error
      if (E_OK != ret) {
        printf("bind fail\n");
        TcpIp_Close(sockId, TRUE);
      }
    }
  }

  if (E_OK == ret) {
    // handle tcp server: listen
    if (IS_CON_TYPE_OF(connection, SOAD_SOCON_TCP_SERVER)) {
      ret = TcpIp_TcpListen(sockId, conG->numOfConnections);
      if (E_OK != ret) {
        TcpIp_Close(sockId, TRUE);
      }
      // handle tcp client: connect
    } else if (IS_CON_TYPE_OF(connection, SOAD_SOCON_TCP_CLIENT)) {
      //printf("going to connect...\n");
      ret = TcpIp_TcpConnect(sockId, &context->RemoteAddr);
      if (E_OK != ret) {
        TcpIp_Close(sockId, TRUE);
      }
    } else {
      /* do nothing */
    }
  }

  if (E_OK == ret) {
    if (IS_CON_TYPE_OF(connection, SOAD_SOCON_TCP_SERVER)) {
      context->state = SOAD_SOCKET_ACCEPT;
    } else {
      context->state = SOAD_SOCKET_READY;
    }
    context->sock = sockId;
    if (conG->SoConModeChgNotification) {
      conG->SoConModeChgNotification(index, SOAD_SOCON_ONLINE);
    }
    SANE_DEBUGF(SANE_DBG_TRACE,("[%d] create TcpIp socket %d, next state %d\n", index, sockId,
           context->state));
  } else {
    SANE_DEBUGF(SANE_DBG_TRACE,("[%d] failed to creat socket with error %d\n", index, sockId));
    context->state = SOAD_SOCKET_CLOSED;
  }
}

void soAdSocketAcceptMain(int index) {
  SoAd_SocketContextType *context = &SOAD_CONFIG->Contexts[index];
  const SoAd_SocketConnectionType *connection =
      &SOAD_CONFIG->Connections[index];
  const SoAd_SocketConnectionGroupType *conG =
      &SOAD_CONFIG->ConnectionGroups[connection->GID];
  const SoAd_TpInterfaceType *IF =
      (const SoAd_TpInterfaceType *)conG->Interface;

  PduInfoType PduInfo;
  PduLengthType bufferSize;
  Std_ReturnType ret;
  TcpIp_SocketIdType SocketId;
  SoAd_SocketContextType *actCtx = NULL;
  const SoAd_SocketConnectionType *actCnt = NULL;
  TcpIp_SockAddrType RemoteAddr;
  int i;

  if (TCPIP_IPPROTO_TCP == conG->ProtocolType) {
    //printf("going to accept!\n");
    ret = TcpIp_TcpAccept(context->sock, &SocketId, &RemoteAddr);
    if (E_OK == ret) {
      ret = E_NOT_OK;
      for (i = 0; i < conG->numOfConnections; i++) {
        if (SOAD_SOCKET_CLOSED ==
            SOAD_CONFIG->Contexts[i + conG->SoConId].state) {
          actCtx = &SOAD_CONFIG->Contexts[i + conG->SoConId];
          actCnt = &SOAD_CONFIG->Connections[i + conG->SoConId];
          actCtx->sock = SocketId;
          actCtx->RemoteAddr = RemoteAddr;
          actCtx->state = SOAD_SOCKET_READY;
          ret = E_OK;
          break;
        }
      }
      if (E_OK != ret) {
        SANE_DEBUGF(SANE_DBG_TRACE,("[%d] accept failed as no free slot\n", index));
        TcpIp_Close(SocketId, TRUE);
      } else {
        SANE_DEBUGF(SANE_DBG_TRACE,("[%d] accept as new socket\n", i + conG->SoConId));
        //printf("i:%d SoConId:%d sock:%d\n", i, conG->SoConId, SocketId);
      }
    }
    if (E_OK == ret) {
      if (conG->SoConModeChgNotification) {
        conG->SoConModeChgNotification(i + conG->SoConId, SOAD_SOCON_ONLINE);
      }
      if (conG->IsTP && IF->TpStartOfReception) {
        PduInfo.SduDataPtr = NULL;
        PduInfo.SduLength = 0;
        PduInfo.MetaDataPtr = (uint8_t *)&actCtx->RemoteAddr;
        IF->TpStartOfReception(actCnt->RxPduId, &PduInfo, 0, &bufferSize);
      }
    }
  } else {
    SANE_DEBUGF(SANE_DBG_TRACE,("UDP cannot accept"));
  }
}

Std_ReturnType soAdSocketTcpReadyMain(SoAd_SoConIdType SoConId, uint8_t *dataIn,
                                      uint32_t length) {
  const SoAd_SocketConnectionType *connection =
      &SOAD_CONFIG->Connections[SoConId];
  const SoAd_SocketConnectionGroupType *conG =
      &SOAD_CONFIG->ConnectionGroups[connection->GID];
  SoAd_SocketContextType *context = &SOAD_CONFIG->Contexts[SoConId];
  Std_ReturnType ret;
  uint32_t maxLength = 1024;
  uint32_t recvLength = maxLength;
  uint8_t *data = NULL;

  ret = TcpIp_IsTcpStatusOK(context->sock);
  if (E_OK == ret) {
    if (NULL == dataIn) {
      // printf("no data here\n");
      // rxLen = TcpIp_Tell(context->sock);
      // printf("rxLen > 0");
      data = malloc(maxLength);
    } else {
      //printf("get data!\n");
      data = dataIn;
      maxLength = length;
    }
       //printf("going to recv!\n");
      ret = TcpIp_Recv(context->sock, data, &recvLength);
      // printf("recved!\n");
      if (E_OK == ret) {
        // printf("[%d] TCP read %d bytes\n", SoConId, rxLen);
        if (recvLength != 0) { //recvLength stores the number of bytes that received
          SANE_DEBUGF(SANE_DBG_TRACE,("data: %s", data));
          if (conG->IsTP) {
            soAdSocketTpRxNotify(context, connection, data, recvLength);
          } else {
            soAdSocketIfRxNotify(context, connection, data, recvLength);
          }
        }
        else {
          ret = E_NOT_OK;
        }
      } else {
        SANE_DEBUGF(SANE_DBG_TRACE,("[%d] TCP read failed\n", SoConId));
      }
      if (data != dataIn) {
        free(data);
      }
  } else {
    TcpIp_Close(context->sock, TRUE);
    if (conG->SoConModeChgNotification) {
      conG->SoConModeChgNotification(SoConId, SOAD_SOCON_OFFLINE);
    }
    context->state = SOAD_SOCKET_CLOSED;
    SANE_DEBUGF(SANE_DBG_TRACE,("[%d] close, goto accept\n", SoConId));
  }
  return ret;
}

Std_ReturnType soAdSocketUdpReadyMain(SoAd_SoConIdType SoConId, uint8_t *dataIn,
                                      uint32_t length) {
  const SoAd_SocketConnectionType *connection =
      &SOAD_CONFIG->Connections[SoConId];
  const SoAd_SocketConnectionGroupType *conG =
      &SOAD_CONFIG->ConnectionGroups[connection->GID];
  SoAd_SocketContextType *context = &SOAD_CONFIG->Contexts[SoConId];
  Std_ReturnType ret = E_OK;
  uint32_t rxLen;
  uint32_t maxLength = 2048;
  uint8_t *data = NULL;
  if (NULL == dataIn) {
    data = malloc(maxLength);
    rxLen = maxLength;
  } else {
    data = dataIn;
    rxLen = length;
  }
  if (E_OK == ret) {
    ret = TcpIp_RecvFrom(context->sock, &context->RemoteAddr, data, &rxLen);
    if (E_OK == ret) {
      if (rxLen > 0) {
        SANE_DEBUGF(SANE_DBG_TRACE,("[%d] UDP read %d bytes\n", SoConId, rxLen));
        if (conG->IsTP) {
          soAdSocketTpRxNotify(context, connection, data, rxLen);
        } else {
          soAdSocketIfRxNotify(context, connection, data, rxLen);
        }
      }
      else {
        ret = E_NOT_OK;
      }
    } else {
      SANE_DEBUGF(SANE_DBG_TRACE,("[%d] UDP read failed\n", SoConId));
    }
    if (data != dataIn) {
      // free the buffer?
        free(data);
    }
  }
  return ret;
}

void soAdSocketReadyMain(int index) {
  const SoAd_SocketConnectionType *connection =
      &SOAD_CONFIG->Connections[index];
  const SoAd_SocketConnectionGroupType *conG =
      &SOAD_CONFIG->ConnectionGroups[connection->GID];
  Std_ReturnType ret = E_OK;

  while (E_OK == ret) {
    if (TCPIP_IPPROTO_TCP == conG->ProtocolType) {
      ret = soAdSocketTcpReadyMain(index, NULL, 0);
    } else {
      ret = soAdSocketUdpReadyMain(index, NULL, 0);
    }
  }
}

// SWS_SoAd_00590
void SoAd_MainFunction() {
  int i;
  SoAd_SocketContextType *context;
  //SANE_DEBUGF(SANE_DBG_STATE,("start SoAd_MainFunction\n"));
  for (i = 0; i < SOAD_CONFIG->numOfConnections; i++) {
    context = &SOAD_CONFIG->Contexts[i];
    switch (context->state) {
      case SOAD_SOCKET_CREATE:
      //printf("context:%d state:create\n", i);
        //SANE_DEBUGF(SANE_DBG_STATE,("context:%d state:create\n", i));
        soAdCreateSocket(i);
        break;
      case SOAD_SOCKET_ACCEPT:
        //SANE_DEBUGF(SANE_DBG_STATE,("context:%d state:accept\n", i));
        soAdSocketAcceptMain(i);
        break;
      case SOAD_SOCKET_READY:
        //SANE_DEBUGF(SANE_DBG_STATE,("context:%d state:ready\n", i));
        soAdSocketReadyMain(i);
        break;
      default:
        break;
    }
  }
}

void SoAd_Init(const SoAd_ConfigType *ConfigPtr) {
  int i;
  const SoAd_SocketConnectionType *connection;
  const SoAd_SocketConnectionGroupType *conG;
  SoAd_SocketContextType *context;

  for (i = 0; i < SOAD_CONFIG->numOfConnections; i++) {
    connection = &SOAD_CONFIG->Connections[i];
    context = &SOAD_CONFIG->Contexts[i];
    context->state = SOAD_SOCKET_CLOSED;
#if SOAD_ERROR_COUNTER_LIMIT > 0
    context->errorCounter = 0;
#endif
    if (connection->GID < SOAD_CONFIG->numOfGroups) {
      conG = &SOAD_CONFIG->ConnectionGroups[connection->GID];
      if ((FALSE == IS_CON_TYPE_OF(connection, SOAD_SOCON_TCP_ACCEPT)) &&
          conG->AutomaticSoConSetup) {
        context->state = SOAD_SOCKET_CREATE;
      }
      TcpIp_SetupAddrFrom(&context->RemoteAddr, conG->Remote, conG->Port);
    } else {
      SANE_DEBUGF(SANE_DBG_TRACE,("[%d] Invalid GID\n", i));
    }
  }
}

Std_ReturnType SoAd_OpenSoCon(SoAd_SoConIdType SoConId) {
  Std_ReturnType ret = E_NOT_OK;
  SoAd_SocketContextType *context;

  if (SoConId < SOAD_CONFIG->numOfConnections) {
    context = &SOAD_CONFIG->Contexts[SoConId];
    if (SOAD_SOCKET_CLOSED == context->state) {
#if SOAD_ERROR_COUNTER_LIMIT > 0
      context->errorCounter = 0;
#endif
      SANE_DEBUGF(SANE_DBG_TRACE,("open soconid:%d\n", SoConId));
      context->state = SOAD_SOCKET_CREATE;
      ret = E_OK;
    } else {
      SANE_DEBUGF(SANE_DBG_TRACE,("[%d] open failed as already in state %d\n", SoConId,
             context->state));
    }
  }

  return ret;
}

Std_ReturnType SoAd_CloseSoCon(SoAd_SoConIdType SoConId, boolean abort) {
  Std_ReturnType ret = E_NOT_OK;
  SoAd_SocketContextType *context;
  const SoAd_SocketConnectionType *connection;
  const SoAd_SocketConnectionGroupType *conG;
  if (SoConId < SOAD_CONFIG->numOfConnections) {
    context = &SOAD_CONFIG->Contexts[SoConId];
    connection = &SOAD_CONFIG->Connections[SoConId];
    conG = &SOAD_CONFIG->ConnectionGroups[connection->GID];
    if (SOAD_SOCKET_CLOSED != context->state) {
      if (conG->SoConModeChgNotification) {
        conG->SoConModeChgNotification(SoConId, SOAD_SOCON_OFFLINE);
      }
      ret = TcpIp_Close(context->sock, abort);
      if (E_OK == ret) {
        context->state = SOAD_SOCKET_CLOSED;
        TcpIp_SetupAddrFrom(&context->RemoteAddr, conG->Remote, conG->Port);
      } else {
        SANE_DEBUGF(SANE_DBG_TRACE,("[%d] close fail: %d\n", SoConId, ret));
      }
    }
  }

  return ret;
}

Std_ReturnType SoAd_IfTransmit(PduIdType TxPduId,
                               const PduInfoType *PduInfoPtr) {
  Std_ReturnType ret = E_NOT_OK;
  SoAd_SoConIdType SoConId;
  const SoAd_SocketConnectionType *connection;
  const SoAd_SocketConnectionGroupType *conG;
  SoAd_SocketContextType *context;
  TcpIp_SockAddrType addr;

  SANE_DEBUGF(SANE_DBG_STATE,("going to transmit TxPduId:%d\n", TxPduId));
  SANE_DEBUGF(SANE_DBG_STATE,("Udp length:%d\n", PduInfoPtr->SduLength));
  if (TxPduId < SOAD_CONFIG->numOfTxPduIds) {
    SoConId = SOAD_CONFIG->TxPduIdToSoCondIdMap[TxPduId];
    connection = &SOAD_CONFIG->Connections[SoConId];
    conG = &SOAD_CONFIG->ConnectionGroups[connection->GID];
    context = &SOAD_CONFIG->Contexts[SoConId];
    SANE_DEBUGF(SANE_DBG_STATE,("TxPduId < SOAD_CONFIG->numOfTxPduIds\n"));
    SANE_DEBUGF(SANE_DBG_STATE,("soconid:%d state:%d\n", SoConId, context->state));
    if (SOAD_SOCKET_READY <= context->state) {
      SANE_DEBUGF(SANE_DBG_STATE,("ready to send\n"));
      if (TCPIP_IPPROTO_UDP == conG->ProtocolType) {
        if (PduInfoPtr->MetaDataPtr != NULL) {
          addr = *(const TcpIp_SockAddrType *)PduInfoPtr->MetaDataPtr;
          ret = TcpIp_SendTo(context->sock, &addr, PduInfoPtr->SduDataPtr,
                             PduInfoPtr->SduLength);
        } else {
          TcpIp_SetupAddrFrom(&addr, conG->Remote, conG->Port);
          ret = TcpIp_SendTo(context->sock, &addr, PduInfoPtr->SduDataPtr,
                             PduInfoPtr->SduLength);
        }
      } else {
        SANE_DEBUGF(SANE_DBG_STATE,("send!: %d\n", context->sock));
        ret = TcpIp_Send(context->sock, PduInfoPtr->SduDataPtr,
                         PduInfoPtr->SduLength);
      }

#if SOAD_ERROR_COUNTER_LIMIT > 0
      if (E_OK != ret) {
        context->errorCounter++;
        if (context->errorCounter >= SOAD_ERROR_COUNTER_LIMIT) {
          SoAd_CloseSoCon(SoConId, TRUE);
        }
      }
#endif
    }
  }

  return ret;
}

//must be tcp?
Std_ReturnType SoAd_TpTransmit(PduIdType TxPduId, const PduInfoType *PduInfoPtr) {
  Std_ReturnType ret = E_NOT_OK;
  SoAd_SoConIdType SoConId;
  const SoAd_SocketConnectionType *connection;
  const SoAd_SocketConnectionGroupType *conG;
  SoAd_SocketContextType *context;

  if (TxPduId < SOAD_CONFIG->numOfTxPduIds) {
    SoConId = SOAD_CONFIG->TxPduIdToSoCondIdMap[TxPduId];
    connection = &SOAD_CONFIG->Connections[SoConId];
    conG = &SOAD_CONFIG->ConnectionGroups[connection->GID];
    context = &SOAD_CONFIG->Contexts[SoConId];
    if (SOAD_SOCKET_READY <= context->state) {
      if (TCPIP_IPPROTO_TCP == conG->ProtocolType) {
        ret = TcpIp_Send(context->sock, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
#if SOAD_ERROR_COUNTER_LIMIT > 0
        if (E_OK != ret) {
          context->errorCounter++;
          if (context->errorCounter >= SOAD_ERROR_COUNTER_LIMIT) {
            SoAd_CloseSoCon(SoConId, TRUE);
          }
        }
#endif
      }
    }
  }

  return ret;
}

Std_ReturnType SoAd_SetRemoteAddr(SoAd_SoConIdType SoConId,
                                  const TcpIp_SockAddrType *RemoteAddrPtr) {
  Std_ReturnType ret = E_NOT_OK;
  SoAd_SocketContextType *context;

  if (SoConId < SOAD_CONFIG->numOfConnections) {
    context = &SOAD_CONFIG->Contexts[SoConId];
    if (SOAD_SOCKET_CLOSED == context->state) {
      context->RemoteAddr = *RemoteAddrPtr;
      ret = E_OK;
    }
  }

  return ret;
}

Std_ReturnType SoAd_GetLocalAddr(SoAd_SoConIdType SoConId, TcpIp_SockAddrType *LocalAddrPtr,
                                 uint8_t *NetmaskPtr, TcpIp_SockAddrType *DefaultRouterPtr) {
  Std_ReturnType ret = E_NOT_OK;
  const SoAd_SocketConnectionType *connection;
  const SoAd_SocketConnectionGroupType *conG;
  SoAd_SocketContextType *context;
  //printf("GetLocal: soconid:%d\nSOAD_CONFIG->numOfConnections:%d\n", SoConId, SOAD_CONFIG->numOfConnections);
  if (SoConId < SOAD_CONFIG->numOfConnections) {
    connection = &SOAD_CONFIG->Connections[SoConId];
    conG = &SOAD_CONFIG->ConnectionGroups[connection->GID];
    context = &SOAD_CONFIG->Contexts[SoConId];
    if (IS_CON_TYPE_OF(connection, SOAD_SOCON_TCP_CLIENT | SOAD_SOCON_TCP_ACCEPT)) {
      /* for TCP client socket */
      if (context->state >= SOAD_SOCKET_READY) {
        SANE_DEBUGF(SANE_DBG_STATE,("for TCP client socket\n"));
        ret = TcpIp_GetLocalAddr(context->sock, LocalAddrPtr);
      }
    } else { /* SOAD_SOCON_UDP_CLIENT */
    SANE_DEBUGF(SANE_DBG_STATE,("for UDP client socket\n"));
      ret = TcpIp_GetIpAddr(conG->LocalAddrId, LocalAddrPtr, NULL, NULL);
      LocalAddrPtr->port = conG->Port;
    }
  }

  return ret;
}

void SoAd_TimerCallback(void *p) { SoAd_MainFunction(); }

void SoAd_task(void *pvParameters)
{
  for (;;)
  {

    SoAd_MainFunction();
    vTaskDelay(3);

  } // for
}

void SoAd_TcpEchoServerCallback(PduIdType RxPduId, const PduInfoType *PduInfoPtr) {
  //printf("SoAd_TcpEchoServerCallback: %d", RxPduId);
  /*if (RxPduId == 0) { //msg from tcpecho client
    SoAd_IfTransmit(1, PduInfoPtr);
  }*/
  SoAd_IfTransmit(2, PduInfoPtr);
}

Std_ReturnType SoAd_GetSoConId(PduIdType TxPduId, SoAd_SoConIdType *SoConIdPtr) {
  Std_ReturnType ret = E_NOT_OK;

  if ((TxPduId < SOAD_CONFIG->numOfTxPduIds) && (NULL != SoConIdPtr)) {
    *SoConIdPtr = SOAD_CONFIG->TxPduIdToSoCondIdMap[TxPduId];
    //printf("get SoConId: %d, pduid:%d", *SoConIdPtr, TxPduId);
    ret = E_OK;
  }

  return ret;
}

Std_ReturnType SoAd_GetRemoteAddr(SoAd_SoConIdType SoConId, TcpIp_SockAddrType *IpAddrPtr) {
  Std_ReturnType ret = E_NOT_OK;
  const SoAd_SocketConnectionType *connection;
  const SoAd_SocketConnectionGroupType *conG;
  SoAd_SocketContextType *context;

  if (SoConId < SOAD_CONFIG->numOfConnections) {
    connection = &SOAD_CONFIG->Connections[SoConId];
    conG = &SOAD_CONFIG->ConnectionGroups[connection->GID];
    context = &SOAD_CONFIG->Contexts[SoConId];
    if (IS_CON_TYPE_OF(connection, SOAD_SOCON_UDP_SERVER) && conG->IsMulitcast) {
      TcpIp_SetupAddrFrom(IpAddrPtr, conG->Remote, conG->Port);
      ret = E_OK;
    } else if (SOAD_SOCKET_READY <= context->state) {
      *IpAddrPtr = context->RemoteAddr;
      ret = E_OK;
    }
  }

  return ret;
}