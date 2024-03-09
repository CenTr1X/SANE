#include "SomeIp.h"
#include "SomeIp_Cfg.h"
#include "printf.h"
#include <FreeRTOS.h>
#include <task.h>
#include <stdlib.h>
#include <string.h>

/* ================================ [ DECLARES  ] ============================================== */
extern const SomeIp_ConfigType SomeIp_Config;
/* ================================ [ MACROS    ] ============================================== */
#define DEF_SQP(T, size)                                                                           \
  static SomeIp_##T##Type someIp##T##Slots[size];                                                          \

#define DEC_SQP(T)                                                                                 \
  SomeIp_##T##Type *var;                                                                           \
  SomeIp_##T##Type *next

#define SQP_FIRST(T)                                                                               \
  do {                                                                                             \
    var = STAILQ_FIRST(&context->pending##T##s);                                                   \
  } while (0)

#define SQP_NEXT()                                                                                 \
  do {                                                                                             \
    next = STAILQ_NEXT(var, entry);                                                                \
  } while (0)

#define SQP_WHILE(T)                                                                               \
  SQP_FIRST(T);                                                                                    \
  while (NULL != var) {                                                                            \
    SQP_NEXT();

#define SQP_WHILE_END()                                                                            \
  var = next;                                                                                      \
  }

#define SQP_ALLOC(T)                                                                               \
  do {                                                                                             \
    var = (SomeIp_##T##Type *)malloc(sizeof(SomeIp_##T##Type));                                          \
  } while (0)

#define SQP_FREE(T)                                                                                \
  do {                                                                                             \
    free((uint8_t *)var);                                                     \
  } while (0)

/* Context Append */
#define SQP_CAPPEND(T)                                                                             \
  do {                                                                                             \
    STAILQ_INSERT_TAIL(&context->pending##T##s, var, entry);                                       \
  } while (0)

/* CRM: context RM */
#define SQP_CRM_AND_FREE(T)                                                                        \
  do {                                                                                             \
    STAILQ_REMOVE(&context->pending##T##s, var, SomeIp_##T##_s, entry);                            \
    free(var);                                                     \
  } while (0)

/* List Append */
#define SQP_LAPPEND(T)                                                                             \
  do {                                                                                             \
    STAILQ_INSERT_TAIL(pending##T##s, var, entry);                                                 \
  } while (0)

/* LRM: list RM */
#define SQP_LRM_AND_FREE(T)                                                                        \
  do {                                                                                             \
    STAILQ_REMOVE(pending##T##s, var, SomeIp_##T##_s, entry);                                      \
    free(var);                                                     \
  } while (0)

#define SQP_CLEAR(T)                                                                               \
  do {                                                                                             \
    SomeIp_##T##Type *var;                                                                         \
    var = STAILQ_FIRST(&context->pending##T##s);                                                   \
    while (NULL != var) {                                                                          \
      STAILQ_REMOVE_HEAD(&context->pending##T##s, entry);                                          \
      free(var);                                                   \
      var = STAILQ_FIRST(&context->pending##T##s);                                                 \
    }                                                                                              \
  } while (0)

#ifndef SOMEIP_ASYNC_REQUEST_MESSAGE_POOL_SIZE
#define SOMEIP_ASYNC_REQUEST_MESSAGE_POOL_SIZE 8
#endif

#ifndef SOMEIP_RX_TP_MESSAGE_POOL_SIZE
#define SOMEIP_RX_TP_MESSAGE_POOL_SIZE 8
#endif

#ifndef SOMEIP_TX_TP_MESSAGE_POOL_SIZE
#define SOMEIP_TX_TP_MESSAGE_POOL_SIZE 8
#endif

#ifndef SOMEIP_TX_TP_EVENT_MESSAGE_POOL_SIZE
#define SOMEIP_TX_TP_EVENT_MESSAGE_POOL_SIZE 8
#endif

#ifndef SOMEIP_WAIT_RESPONSE_MESSAGE_POOL_SIZE
#define SOMEIP_WAIT_RESPONSE_MESSAGE_POOL_SIZE 8
#endif

#ifndef SOMEIP_TX_NOK_RETRY_MAX
#define SOMEIP_TX_NOK_RETRY_MAX 3
#endif

/* @PRS_SOMEIP_00367 */
#define SOMEIP_TP_FLAG 0x20

#define SOMEIP_SF_MAX 1396
#define SOMEIP_TP_MAX 1392

#define SD_FLG_EVENT_GROUP_MULTICAST 0x10u

/* @SWS_SomeIpXf_00031 */
#define SOMEIP_MSG_REQUEST 0x00
#define SOMEIP_MSG_REQUEST_NO_RETURN 0x01
#define SOMEIP_MSG_NOTIFICATION 0x02
#define SOMEIP_MSG_RESPONSE 0x80
#define SOMEIP_MSG_ERROR 0x81

#define SOMEIP_MSG_REQUEST_ACK 0x40
#define SOMEIP_MSG_REQUEST_NO_RETURN_ACK 0x41
#define SOMEIP_MSG_NOTIFICATION_ACK 0x42

#define SOMEIP_MSG_RESPONSE_ACK 0xC0
#define SOMEIP_MSG_ERROR_ACK 0xC1

#define IS_TP_ENABLED(obj) (NULL != (obj)->onTpCopyTxData)

#define SOMEIP_CONFIG (&SomeIp_Config)
/* ================================ [ DATAS     ] ============================================== */
DEF_SQP(AsyncReqMsg, SOMEIP_ASYNC_REQUEST_MESSAGE_POOL_SIZE)
DEF_SQP(RxTpMsg, SOMEIP_RX_TP_MESSAGE_POOL_SIZE)
DEF_SQP(TxTpMsg, SOMEIP_TX_TP_MESSAGE_POOL_SIZE)
#define someIpRxTpEvtMsgPool someIpRxTpMsgPool
DEF_SQP(TxTpEvtMsg, SOMEIP_TX_TP_EVENT_MESSAGE_POOL_SIZE)
DEF_SQP(WaitResMsg, SOMEIP_WAIT_RESPONSE_MESSAGE_POOL_SIZE)

/* ================================ [ FUNCTIONS ] ============================================== */
Std_ReturnType SomeIp_ResolveSubscriber(uint16_t ServiceId, Sd_EventHandlerSubscriberType *sub) {
  Std_ReturnType ret = E_OK;
  printf("SomeIp_ResolveSubscriber called\n");
  return ret;
}

void onSubscribe_common(uint16_t eventGroupId, TcpIp_SockAddrType* RemoteAddr) {
  printf("RemoteAddr %d.%d.%d.%d subscribe eventgroup %d\n", RemoteAddr->addr[0], RemoteAddr->addr[1], RemoteAddr->addr[2], RemoteAddr->addr[3], eventGroupId);
}

static void SomeIp_InitServer(const SomeIp_ServerServiceType *config) {
  int i;
  for (i = 0; i < config->numOfConnections; i++) {
    memset(config->connections[i].context, 0, sizeof(SomeIp_ServerConnectionContextType));
    STAILQ_INIT(&(config->connections[i].context->pendingAsyncReqMsgs));
    STAILQ_INIT(&(config->connections[i].context->pendingRxTpMsgs));
    STAILQ_INIT(&(config->connections[i].context->pendingTxTpMsgs));
    if (NULL != config->connections[i].tcpBuf) {
      memset(config->connections[i].tcpBuf, 0, sizeof(SomeIp_TcpBufferType));
    }
  }
  memset(config->context, 0, sizeof(SomeIp_ServerContextType));
  STAILQ_INIT(&(config->context->pendingTxTpEvtMsgs));
}

static void SomeIp_InitClient(const SomeIp_ClientServiceType *config) {
  SomeIp_ClientServiceContextType *context = config->context;
  memset(context, 0, sizeof(SomeIp_ClientServiceContextType));
  STAILQ_INIT(&(context->pendingRxTpEvtMsgs));
  STAILQ_INIT(&(context->pendingRxTpMsgs));
  STAILQ_INIT(&(context->pendingTxTpMsgs));
  STAILQ_INIT(&(context->pendingWaitResMsgs));
  if (NULL != config->tcpBuf) {
    memset(config->tcpBuf, 0, sizeof(SomeIp_TcpBufferType));
  }
}

void SomeIp_Init(const SomeIp_ConfigType *ConfigPtr) {
  int i;
  /*SQP_INIT(AsyncReqMsg);
  SQP_INIT(TxTpMsg);
  SQP_INIT(RxTpMsg);
  SQP_INIT(TxTpEvtMsg);
  SQP_INIT(WaitResMsg);*/
  for (i = 0; i < SOMEIP_CONFIG->numOfService; i++) {
    if (SOMEIP_CONFIG->services[i].isServer) {
      SomeIp_InitServer((const SomeIp_ServerServiceType *)SOMEIP_CONFIG->services[i].service);
    } else {
      SomeIp_InitClient((const SomeIp_ClientServiceType *)SOMEIP_CONFIG->services[i].service);
    }
  }
}

static void SomeIp_BuildHeader(uint8_t *header, uint16_t serviceId, uint16_t methodId,
                               uint16_t clientId, uint16_t sessionId, uint8_t interfaceVersion,
                               uint8_t messageType, uint8_t returnCode, uint32_t payloadLength) {
  uint32_t length = payloadLength + 8;
  header[0] = (serviceId >> 8) & 0xFF;
  header[1] = serviceId & 0xFF;
  header[2] = (methodId >> 8) & 0xFF;
  header[3] = methodId & 0xFF;
  header[4] = (length >> 24) & 0xFF;
  header[5] = (length >> 16) & 0xFF;
  header[6] = (length >> 8) & 0xFF;
  header[7] = length & 0xFF;
  header[8] = (clientId >> 8) & 0xFF;
  header[9] = clientId & 0xFF;
  header[10] = (sessionId >> 8) & 0xFF;
  header[11] = sessionId & 0xFF;
  header[12] = 1;
  header[13] = interfaceVersion;
  header[14] = messageType;
  header[15] = returnCode;

  // printf("build: service 0x%x:0x%x:%d message type %d return code %d payload %d bytes from "
  //                "client 0x%x:%d\n",
  //                serviceId, methodId, interfaceVersion, messageType, returnCode, payloadLength,
  //                clientId, sessionId);
}

static Std_ReturnType SomeIp_SendNextTxTpMsg(PduIdType TxPduId, uint16_t serviceId,
                                             uint16_t methodId, uint8_t interfaceVersion,
                                             uint8_t messageType,
                                             SomeIp_OnTpCopyTxDataFncType onTpCopyTxData,
                                             SomeIp_TxTpMsgType *var) {
  Std_ReturnType ret = E_OK;
  SomeIp_TpMessageType tpMsg;
  uint32_t len = var->length - var->offset;
  uint32_t offset = var->offset;
  uint32_t requestId;
  uint8_t *data;
  printf("call SomeIp_SendNextTxTpMsg!");
  if (len > SOMEIP_TP_MAX) {
    len = SOMEIP_TP_MAX;
    offset |= 0x01; /* setup more flag */
    tpMsg.moreSegmentsFlag = TRUE;
  } else {
    tpMsg.moreSegmentsFlag = FALSE;
  }

  data = malloc(len + 20);
  if (NULL != data) {
    tpMsg.data = &data[20];
    tpMsg.length = len;
    tpMsg.offset = var->offset;
    requestId = ((uint32_t)var->clientId << 16) + var->sessionId;
    ret = onTpCopyTxData(requestId, &tpMsg);
    if (E_OK == ret) {
      data[16] = (offset >> 24) & 0xFF;
      data[17] = (offset >> 16) & 0xFF;
      data[18] = (offset >> 8) & 0xFF;
      data[19] = offset & 0xFF;
      ret = SomeIp_Transmit(TxPduId, &var->RemoteAddr, data, serviceId, methodId, var->clientId,
                            var->sessionId, interfaceVersion, messageType | SOMEIP_TP_FLAG, E_OK,
                            len + 4);
      if (E_OK == ret) {
        var->offset += len;
#ifndef DISABLE_SOMEIP_TX_NOK_RETRY
      } else if ((E_NOT_OK == ret) && (var->retryCounter < SOMEIP_TX_NOK_RETRY_MAX)) {
        var->retryCounter++;
        printf("Tx TP NOK, try %d\n", var->retryCounter);
        ret = E_OK;
#endif
      } else {
        printf("Tx TP NOK\n");
      }
    }
  } else {
    printf("OoM, schedule Tx TP msg next time\n");
    ret = E_OK;
  }

  if (NULL != data) {
    free(data);
  }

  return ret;
}

static Std_ReturnType SomeIp_Transmit(PduIdType TxPduId, TcpIp_SockAddrType *RemoteAddr,
                                      uint8_t *data, uint16_t serviceId, uint16_t methodId,
                                      uint16_t clientId, uint16_t sessionId,
                                      uint8_t interfaceVersion, uint8_t messageType,
                                      uint8_t returnCode, uint32_t payloadLength) {
  Std_ReturnType ret;
  PduInfoType PduInfo;

  PduInfo.MetaDataPtr = (uint8_t *)RemoteAddr;
  PduInfo.SduDataPtr = data;
  PduInfo.SduLength = payloadLength + 16;
  SomeIp_BuildHeader(data, serviceId, methodId, clientId, sessionId, interfaceVersion, messageType,
                     returnCode, payloadLength);
  ret = SoAd_IfTransmit(TxPduId, &PduInfo);
  if (E_OK != ret) {
    //printf("Fail to send msg\n");
  } else {
    //printf("SomeIp Transmit success\n");
  }
  return ret;
}

static Std_ReturnType SomeIp_WaitResponse(const SomeIp_ClientServiceType *config, uint16_t methodId,
                                          uint16_t sessionId) {
  Std_ReturnType ret = E_OK;
  SomeIp_ClientServiceContextType *context = config->context;
  SomeIp_WaitResMsgType *var;
  printf("should wait!!!!");
  SQP_ALLOC(WaitResMsg);
  if (NULL != var) {
    var->methodId = methodId;
    var->sessionId = sessionId;
    var->timer = config->ResponseTimeout;
    SQP_CAPPEND(WaitResMsg);
  } else {
    printf("OoM for wait res msg\n");
    ret = E_NOT_OK;
  }
  return ret;
}

static Std_ReturnType SomeIp_SendRequest(const SomeIp_ClientServiceType *config, uint16_t methodIndex,
                                         uint16_t clientId, uint16_t sessionId,
                                         TcpIp_SockAddrType *RemoteAddr, SomeIp_MessageType *req,
                                         uint8_t messageType) {
  Std_ReturnType ret = E_OK;
  SomeIp_ClientServiceContextType *context = config->context;
  const SomeIp_ClientMethodType *method = &config->methods[methodIndex];
  SomeIp_TxTpMsgType *var;
  uint8_t *data;

  if (IS_TP_ENABLED(method) && (req->length > SOMEIP_SF_MAX)) {
    SQP_ALLOC(TxTpMsg);
    if (NULL != var) {
      var->clientId = clientId;
      var->sessionId = sessionId;
      var->methodId = methodIndex;
      var->RemoteAddr = *RemoteAddr;
      var->offset = 0;
      var->length = req->length;
      var->messageType = messageType;
#ifndef DISABLE_SOMEIP_TX_NOK_RETRY
      var->retryCounter = 0;
#endif
      var->timer = config->SeparationTime;
      ret =
        SomeIp_SendNextTxTpMsg(config->TxPduId, config->serviceId, method->methodId,
                               method->interfaceVersion, messageType, method->onTpCopyTxData, var);
      if (E_OK == ret) {
        SQP_CAPPEND(TxTpMsg);
      } else {
        SQP_FREE(TxTpMsg);
      }
    } else {
      ret = SOMEIP_E_NOMEM;
      printf("OoM for cache Tp Tx\n");
    }
  } else {
    data = malloc(req->length + 16);
    if (NULL != data) {
      memcpy(&data[16], req->data, req->length);
      ret = SomeIp_Transmit(config->TxPduId, RemoteAddr, data, config->serviceId, method->methodId,
                            clientId, sessionId, method->interfaceVersion, messageType, E_OK,
                            req->length);
      free(data);
      if (E_OK == ret && messageType == SOMEIP_MSG_REQUEST) {
        ret = SomeIp_WaitResponse(config, methodIndex, sessionId);
      }
    }
  }

  return ret;
}

static Std_ReturnType SomeIp_RequestOrFire(uint32_t requestId, uint8_t *data, uint32_t length,
                                           uint8_t messageType) {
  Std_ReturnType ret = E_OK;
  uint16_t TxMethodId = (requestId >> 16) & 0xFFFF;
  uint16_t sessionId = requestId & 0xFFFF;
  const SomeIp_ClientServiceType *config;
  SomeIp_ClientServiceContextType *context;
  uint16_t index;
  TcpIp_SockAddrType RemoteAddr;
  SomeIp_MessageType msg;
  //printf("going to requestorfire\n TxMethodId:%d sessionId:%d\n", TxMethodId, sessionId);
  if (TxMethodId < SOMEIP_CONFIG->numOfTxMethods) {
    index = SOMEIP_CONFIG->TxMethod2ServiceMap[TxMethodId];
    config = (const SomeIp_ClientServiceType *)SOMEIP_CONFIG->services[index].service;
    context = config->context;
    if (FALSE == context->online) {
      printf("FALSE == context->online\n");
      ret = E_NOT_OK;
    }
  } else {
    ret = E_NOT_OK;
  }

  if (E_OK == ret) {
    index = SOMEIP_CONFIG->TxMethod2PerServiceMap[TxMethodId];
    ret = Sd_GetProviderAddr(config->sdHandleID, &RemoteAddr);
    if (E_OK == ret) {
      msg.data = data;
      msg.length = length;
    }
  }

  if (E_OK == ret) {
    //printf("going to send request \nclientId:%d sessionid:%d msgtype:%d\n", config->clientId, sessionId, messageType);
    ret = SomeIp_SendRequest(config, index, config->clientId, sessionId, &RemoteAddr, &msg,
                             messageType);
  }

  return ret;
}

static Std_ReturnType SomeIp_SendNotification(const SomeIp_ServerServiceType *config,
                                              uint16_t eventId, uint16_t sessionId,
                                              SomeIp_MessageType *req,
                                              Sd_EventHandlerSubscriberListType *list) {
  Std_ReturnType ret = E_OK;
  SomeIp_ServerContextType *context = config->context;
  const SomeIp_ServerEventType *event =
    &config->events[SOMEIP_CONFIG->TxEvent2PerServiceMap[eventId]];
  SomeIp_TxTpEvtMsgType *var;
  uint8_t *data;

  if (IS_TP_ENABLED(event) && (req->length > SOMEIP_SF_MAX)) {
    /*SQP_ALLOC(TxTpEvtMsg);
    if (NULL != var) {
      var->eventId = eventId;
      var->sessionId = sessionId;
      var->offset = 0;
      var->length = req->length;
      var->timer = config->SeparationTime;
      ret = SomeIp_SendNextTxTpEvtMsg(config, event, var, list);
      if (E_OK == ret) {
        SQP_CAPPEND(TxTpEvtMsg);
      } else {
        SQP_FREE(TxTpEvtMsg);
      }
    } else {
      ret = SOMEIP_E_NOMEM;
    }*/
    printf("oh no, its IS_TP_ENABLED\n");
  } else {
    data = malloc(req->length + 16);
    if (NULL != data) {
      memcpy(&data[16], req->data, req->length);
      ret = SomeIp_TransmitEvtMsg(config, event, data, req->length, FALSE, list, sessionId);
      free(data);
    }
  }
}

Std_ReturnType SomeIp_Request(uint32_t requestId, uint8_t *data, uint32_t length) {
  return SomeIp_RequestOrFire(requestId, data, length, SOMEIP_MSG_REQUEST);
}

Std_ReturnType SomeIp_FireForgot(uint32_t requestId, uint8_t *data, uint32_t length) {
  return SomeIp_RequestOrFire(requestId, data, length, SOMEIP_MSG_REQUEST_NO_RETURN);
}

Std_ReturnType SomeIp_Notification(uint32_t requestId, uint8_t *data, uint32_t length) {
  Std_ReturnType ret = E_OK;
  uint16_t TxEventId = (requestId >> 16) & 0xFFFF;
  uint16_t sessionId = requestId & 0xFFFF;
  const SomeIp_ServerServiceType *config;
  const SomeIp_ServerEventType *event;
  uint16_t index;
  Sd_EventHandlerSubscriberListType *list;
  SomeIp_MessageType msg;

  if (TxEventId < SOMEIP_CONFIG->numOfTxEvents) {
    index = SOMEIP_CONFIG->TxEvent2ServiceMap[TxEventId];
    config = (const SomeIp_ServerServiceType *)SOMEIP_CONFIG->services[index].service;
    if (FALSE == config->context->online) {
      ret = E_NOT_OK;
    }
  } else {
    ret = E_NOT_OK;
  }

  if (E_OK == ret) {
    index = SOMEIP_CONFIG->TxEvent2PerServiceMap[TxEventId];
    event = &config->events[index];
    ret = Sd_GetSubscribers(event->sdHandleID, &list);
  }

  if (E_OK == ret) {
    msg.data = data;
    msg.length = length;
  }

  if (E_OK == ret) {
    ret = SomeIp_SendNotification(config, TxEventId, sessionId, &msg, list);
  }

  return ret;
}

static Std_ReturnType SomeIp_TransError(PduIdType TxPduId, TcpIp_SockAddrType *RemoteAddr,
                                        uint16_t serviceId, uint16_t methodId, uint16_t clientId,
                                        uint16_t sessionId, uint8_t interfaceVersion,
                                        uint8_t messageType, uint8_t returnCode) {
  Std_ReturnType ret;
  PduInfoType PduInfo;
  uint8_t errMsg[16];

  PduInfo.MetaDataPtr = (uint8_t *)RemoteAddr;
  PduInfo.SduDataPtr = errMsg;
  PduInfo.SduLength = 16;
  SomeIp_BuildHeader(errMsg, serviceId, methodId, clientId, sessionId, interfaceVersion,
                     messageType, returnCode, 0);
  ret = SoAd_IfTransmit(TxPduId, &PduInfo);
  if (E_OK != ret) {
    printf("Fail to send error\n");
  } else {
    printf("SomeIp Transmit Error success\n");
  }
  return ret;
}

static SomeIp_RxTpMsgType *SomeIp_RxTpMsgFind(SomeIp_RxTpMsgList *pendingRxTpMsgs,
                                              uint16_t methodId) {
  SomeIp_RxTpMsgType *rxTpMsg = NULL;
  SomeIp_RxTpMsgType *var;
  STAILQ_FOREACH(var, pendingRxTpMsgs, entry) {
    if (var->methodId == methodId) {
      rxTpMsg = var;
      break;
    }
  }
  return rxTpMsg;
}

static Std_ReturnType SomeIp_ReplyRequest(const SomeIp_ServerServiceType *config, uint16_t conId,
                                          uint16_t methodId, uint16_t clientId, uint16_t sessionId,
                                          TcpIp_SockAddrType *RemoteAddr, SomeIp_MessageType *res) {
  Std_ReturnType ret = E_OK;
  const SomeIp_ServerConnectionType *connection = &config->connections[conId];
  SomeIp_ServerConnectionContextType *context = connection->context;
  const SomeIp_ServerMethodType *method = &config->methods[methodId];
  SomeIp_TxTpMsgType *var;
  printf("SomeIp_ReplyRequest: res->length:%d", res->length);
  if (IS_TP_ENABLED(method) && (res->length > SOMEIP_SF_MAX)) {
    SQP_ALLOC(TxTpMsg);
    if (NULL != var) {
      var->clientId = clientId;
      var->sessionId = sessionId;
      var->methodId = methodId;
      var->RemoteAddr = *RemoteAddr;
      var->offset = 0;
      var->length = res->length;
#ifndef DISABLE_SOMEIP_TX_NOK_RETRY
      var->retryCounter = 0;
#endif
      var->timer = config->SeparationTime;
      ret = SomeIp_SendNextTxTpMsg(connection->TxPduId, config->serviceId, method->methodId,
                                   method->interfaceVersion, SOMEIP_MSG_RESPONSE,
                                   method->onTpCopyTxData, var);
      if (E_OK == ret) {
        SQP_CAPPEND(TxTpMsg);
      } else {
        SQP_FREE(TxTpMsg);
      }
    } else {
      ret = SOMEIP_E_NOMEM;
      printf("OoM for cache Tp Tx\n");
    }
  } else {
    ret = SomeIp_Transmit(connection->TxPduId, RemoteAddr, res->data, config->serviceId,
                          method->methodId, clientId, sessionId, method->interfaceVersion,
                          SOMEIP_MSG_RESPONSE, E_OK, res->length);
  }

  return ret;
}

static Std_ReturnType SomeIp_ReplyError(PduIdType TxPduId, SomeIp_MsgType *msg, uint8_t errorCode) {
  Std_ReturnType ret = SomeIp_TransError(
    TxPduId, &msg->RemoteAddr, msg->header.serviceId, msg->header.methodId, msg->header.clientId,
    msg->header.sessionId, msg->header.interfaceVersion, SOMEIP_MSG_ERROR, errorCode);
  return ret;
}

static Std_ReturnType SomeIp_ProcessRequest(const SomeIp_ServerServiceType *config, uint16_t conId,
                                            uint16_t methodId, SomeIp_MsgType *msg) {
  Std_ReturnType ret = E_OK;
  uint8_t *resData = NULL;
  const SomeIp_ServerConnectionType *connection = &config->connections[conId];
  SomeIp_ServerConnectionContextType *context = connection->context;
  const SomeIp_ServerMethodType *method = &config->methods[methodId];
  SomeIp_AsyncReqMsgType *var;
  SomeIp_MessageType res;
  uint32_t requestId = ((uint32_t)msg->header.clientId << 16) + msg->header.sessionId;
  printf("SomeIp_ProcessRequest\n");
  resData = malloc(method->resMaxLen + 16); //1404+16
  if (NULL == resData) {
    printf("OoM for client request\n");
    ret = SOMEIP_E_NOMEM;
  } else {
    res.data = &resData[16];
    res.length = method->resMaxLen;//1404
    ret = method->onRequest(requestId, &msg->req, &res);
    res.data = resData;
  }

  if (E_OK == ret) {
    if (IS_TP_ENABLED(method) && (res.length > SOMEIP_SF_MAX)) {
      free(resData);
      resData = NULL;
    }
  }

  if (E_OK == ret) {
    ret = SomeIp_ReplyRequest(config, conId, methodId, msg->header.clientId, msg->header.sessionId,
                              &msg->RemoteAddr, &res);
  } else if (SOMEIP_E_PENDING == ret) {
    /* response pending */
    SQP_ALLOC(AsyncReqMsg);
    if (NULL != var) {
      var->clientId = msg->header.clientId;
      var->sessionId = msg->header.sessionId;
      var->methodId = methodId;
      var->RemoteAddr = msg->RemoteAddr;
      SQP_CAPPEND(AsyncReqMsg);
    } else {
      ret = SOMEIP_E_NOMEM;
      printf("OoM for cache request\n");
    }
  }

  if (NULL != resData) {
    free(resData);
  }

  return ret;
}

static Std_ReturnType SomeIp_IsResponseExpected(const SomeIp_ClientServiceType *config,
                                                uint16_t methodId, SomeIp_MsgType *msg) {
  Std_ReturnType ret = E_NOT_OK;
  SomeIp_ClientServiceContextType *context = config->context;
  SomeIp_WaitResMsgType *var;
  if ((FALSE == msg->header.isTpFlag) || (0 == msg->tpHeader.offset)) {
    STAILQ_FOREACH(var, &context->pendingWaitResMsgs, entry) {
      if ((var->methodId == methodId) && (var->sessionId == msg->header.sessionId)) {
        ret = E_OK;
        SQP_CRM_AND_FREE(WaitResMsg);
        break;
      }
    }
  } else { /* For TP msg with offset not 0, no check */
    ret = E_OK;
  }

  return ret;
}

static Std_ReturnType SomeIp_TransmitEvtMsg(const SomeIp_ServerServiceType *config,
                                            const SomeIp_ServerEventType *event, uint8_t *data,
                                            uint32_t payloadLength, bool isTp,
                                            Sd_EventHandlerSubscriberListType *list,
                                            uint16_t sessionId) {
  Std_ReturnType ret = E_NOT_OK;
  Std_ReturnType ret2;
  Sd_EventHandlerSubscriberType *sub;
  uint8_t messageType = SOMEIP_MSG_NOTIFICATION;
  TcpIp_SockAddrType *RemoteAddr = NULL;
  bool multicasted = FALSE;

  if (isTp) {
    messageType |= SOMEIP_TP_FLAG;
  }

  sub = STAILQ_FIRST(list);
  while (NULL != sub) {
    if (sub->flags) {
      if (0 == (SD_FLG_EVENT_GROUP_MULTICAST & sub->flags)) {
        RemoteAddr = &sub->RemoteAddr;
        if (FALSE == multicasted) {
          multicasted = TRUE;
        } else {
          continue;
        }
      }
      ret2 = SomeIp_Transmit(sub->TxPduId, RemoteAddr, data, config->serviceId, event->eventId,
                             config->clientId, sessionId, event->interfaceVersion, messageType, 0,
                             payloadLength);
      if (E_OK == ret2) {
        ret = E_OK;
      } else {
        printf("Failed to notify event %x:%x to %d.%d.%d.%d:%d\n", config->serviceId,
                        event->eventId, sub->RemoteAddr.addr[0], sub->RemoteAddr.addr[1],
                        sub->RemoteAddr.addr[2], sub->RemoteAddr.addr[3], sub->RemoteAddr.port);
      }
    }
    /* NOTE: if any one is unsubscribed during this, TX is broken */
    sub = STAILQ_NEXT(sub, entry);
  }

  return ret;
}

static Std_ReturnType SomeIp_SendNextTxTpEvtMsg(const SomeIp_ServerServiceType *config,
                                                const SomeIp_ServerEventType *event,
                                                SomeIp_TxTpEvtMsgType *var,
                                                Sd_EventHandlerSubscriberListType *list) {
  Std_ReturnType ret = E_OK;
  SomeIp_TpMessageType tpMsg;
  uint32_t len = var->length - var->offset;
  uint32_t offset = var->offset;
  uint8_t *data;
  uint32_t requestId;

  if (len > SOMEIP_TP_MAX) {
    len = SOMEIP_TP_MAX;
    offset |= 0x01; /* setup more flag */
    tpMsg.moreSegmentsFlag = TRUE;
  } else {
    tpMsg.moreSegmentsFlag = FALSE;
  }

  data = malloc(len + 20);
  if (NULL != data) {
    tpMsg.data = &data[20];
    tpMsg.length = len;
    tpMsg.offset = var->offset;
    requestId = ((uint32_t)var->eventId << 16) + var->sessionId;
    ret = event->onTpCopyTxData(requestId, &tpMsg);
    if (E_OK == ret) {
      data[16] = (offset >> 24) & 0xFF;
      data[17] = (offset >> 16) & 0xFF;
      data[18] = (offset >> 8) & 0xFF;
      data[19] = offset & 0xFF;
      ret = SomeIp_TransmitEvtMsg(config, event, data, len + 4, TRUE, list, var->sessionId);
      if (E_OK == ret) {
        var->offset += len;
      } else {
        printf("Tx TP EVT NOK\n");
      }
    }
  } else {
    printf("OoM, schedule Tx TP EVT msg next time\n");
    ret = E_OK;
  }

  if (NULL != data) {
    free(data);
  }

  return ret;
}

static void SomeIp_MainServerAsyncRequest(const SomeIp_ServerServiceType *config, uint16_t conId) {
  const SomeIp_ServerConnectionType *connection = &config->connections[conId];
  SomeIp_ServerConnectionContextType *context = connection->context;
  DEC_SQP(AsyncReqMsg);
  const SomeIp_ServerMethodType *method = NULL;
  uint8_t *resData;
  SomeIp_MessageType res;
  Std_ReturnType ret;

  SQP_WHILE(AsyncReqMsg) {
    method = &config->methods[var->methodId];
    resData = malloc(method->resMaxLen + 16);
    if (NULL != resData) {
      res.data = &resData[16];
      res.length = method->resMaxLen;
      ret = method->onAsyncRequest(conId, &res);
      res.data = resData;
      if (E_OK == ret) {
        ret = SomeIp_ReplyRequest(config, conId, var->methodId, var->clientId, var->sessionId,
                                  &var->RemoteAddr, &res);
        if (E_OK != ret) {
          (void)SomeIp_TransError(connection->TxPduId, &var->RemoteAddr, config->serviceId,
                                  method->methodId, var->clientId, var->sessionId,
                                  method->interfaceVersion, SOMEIP_MSG_ERROR, ret);
        }

        SQP_CRM_AND_FREE(AsyncReqMsg);
      } else if (SOMEIP_E_PENDING == ret) {
        /* do nothing */
      } else {
        (void)SomeIp_TransError(connection->TxPduId, &var->RemoteAddr, config->serviceId,
                                method->methodId, var->clientId, var->sessionId,
                                method->interfaceVersion, SOMEIP_MSG_RESPONSE, ret);

        SQP_CRM_AND_FREE(AsyncReqMsg);
      }
      free(resData);
    } else {
      printf("OoM for async request\n");
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainServerRxTpMsg(const SomeIp_ServerServiceType *config, uint16_t conId) {
  const SomeIp_ServerConnectionType *connection = &config->connections[conId];
  SomeIp_ServerConnectionContextType *context = connection->context;
  DEC_SQP(RxTpMsg);
  const SomeIp_ServerMethodType *method = NULL;
  Std_ReturnType ret;

  SQP_WHILE(RxTpMsg) {
    method = &config->methods[var->methodId];

    if (var->timer > 0) {
      var->timer--;
      if (0 == var->timer) {
        printf("server method %x:%x:%x:%d Rx Tp msg timeout, offset %d\n", config->serviceId,
               method->methodId, var->clientId, var->sessionId, var->offset);
        ret = SomeIp_TransError(connection->TxPduId, &var->RemoteAddr, config->serviceId,
                                method->methodId, var->clientId, var->sessionId,
                                method->interfaceVersion, SOMEIP_MSG_ERROR, SOMEIPXF_E_TIMEOUT);
        if (E_NOT_OK == ret) {
          var->timer = 1; /* retry next time */
        } else {
          method->onTpCopyRxData(((uint32_t)var->clientId << 16) + var->sessionId, NULL);
          SQP_CRM_AND_FREE(RxTpMsg);
        }
      }
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainServerTxTpMsg(const SomeIp_ServerServiceType *config, uint16_t conId) {
  const SomeIp_ServerConnectionType *connection = &config->connections[conId];
  SomeIp_ServerConnectionContextType *context = connection->context;
  DEC_SQP(TxTpMsg);
  const SomeIp_ServerMethodType *method = NULL;
  Std_ReturnType ret;

  SQP_WHILE(TxTpMsg) {
    method = &config->methods[var->methodId];
    if (var->timer > 0) {
      var->timer--;
    }
    if (0 == var->timer) {
      ret = SomeIp_SendNextTxTpMsg(connection->TxPduId, config->serviceId, method->methodId,
                                   method->interfaceVersion, SOMEIP_MSG_RESPONSE,
                                   method->onTpCopyTxData, var);
      if (E_OK == ret) {
        if (var->offset >= var->length) {
          SQP_CRM_AND_FREE(TxTpMsg);
        } else {
          var->timer = config->SeparationTime;
        }
      } else { /* abort this tx */
        (void)method->onTpCopyTxData(((uint32_t)var->clientId << 16) + var->sessionId, NULL);
        SQP_CRM_AND_FREE(TxTpMsg);
      }
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainServerTxTpEvtMsg(const SomeIp_ServerServiceType *config) {
  SomeIp_ServerContextType *context = config->context;
  DEC_SQP(TxTpEvtMsg);
  const SomeIp_ServerEventType *event = NULL;
  Sd_EventHandlerSubscriberListType *list;
  Std_ReturnType ret;

  SQP_WHILE(TxTpEvtMsg) {
    event = &config->events[SOMEIP_CONFIG->TxEvent2PerServiceMap[var->eventId]];
    if (var->timer > 0) {
      var->timer--;
    }
    if (0 == var->timer) {
      /* NOTE: may result partial data send to later online subscribers */
      ret = Sd_GetSubscribers(event->sdHandleID, &list);
      if (E_OK == ret) {
        ret = SomeIp_SendNextTxTpEvtMsg(config, event, var, list);
        if (E_OK == ret) {
          if (var->offset >= var->length) {
            SQP_CRM_AND_FREE(TxTpEvtMsg);
          } else {
            var->timer = config->SeparationTime;
          }
        } else { /* abort this tx */
          (void)event->onTpCopyTxData(((uint32_t)var->eventId << 16) + var->sessionId, NULL);
          SQP_CRM_AND_FREE(TxTpEvtMsg);
        }
      }
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainServer(const SomeIp_ServerServiceType *config) {
  uint16_t conId;

  for (conId = 0; conId < config->numOfConnections; conId++) {
    SomeIp_MainServerAsyncRequest(config, conId);
    SomeIp_MainServerTxTpMsg(config, conId);
    SomeIp_MainServerRxTpMsg(config, conId);
  }

  SomeIp_MainServerTxTpEvtMsg(config);
}

static void SomeIp_MainClientRxTpMsg(const SomeIp_ClientServiceType *config) {
  SomeIp_ClientServiceContextType *context = config->context;
  DEC_SQP(RxTpMsg);
  const SomeIp_ClientMethodType *method = NULL;
  uint32_t requestId;

  SQP_WHILE(RxTpMsg) {
    method = &config->methods[var->methodId];

    if (var->timer > 0) {
      var->timer--;
      if (0 == var->timer) {
        requestId = ((uint32_t)var->clientId << 16) + var->sessionId;
        printf("client method %x:%x:%x:%d Rx Tp msg timeout, offset %d\n", config->serviceId,
               method->methodId, var->clientId, var->sessionId, var->offset);
        method->onError(requestId, SOMEIPXF_E_TIMEOUT);
        SQP_CRM_AND_FREE(RxTpMsg);
      }
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainClientRxTpEvtMsg(const SomeIp_ClientServiceType *config) {
  SomeIp_ClientServiceContextType *context = config->context;
  DEC_SQP(RxTpEvtMsg);

  SQP_WHILE(RxTpEvtMsg) {
    if (var->timer > 0) {
      var->timer--;
      if (0 == var->timer) {
        printf("client event %x:%x:%x:%d Rx Tp msg timeout, offset %d\n", config->serviceId,
               config->events[var->methodId].eventId, var->clientId, var->sessionId, var->offset);
        SQP_CRM_AND_FREE(RxTpEvtMsg);
      }
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainClientTxTpMsg(const SomeIp_ClientServiceType *config) {
  SomeIp_ClientServiceContextType *context = config->context;
  DEC_SQP(TxTpMsg);
  const SomeIp_ClientMethodType *method = NULL;
  Std_ReturnType ret;

  SQP_WHILE(TxTpMsg) {
    method = &config->methods[var->methodId];
    if (var->timer > 0) {
      var->timer--;
    }
    if (0 == var->timer) {
      ret = SomeIp_SendNextTxTpMsg(config->TxPduId, config->serviceId, method->methodId,
                                   method->interfaceVersion, SOMEIP_MSG_REQUEST,
                                   method->onTpCopyTxData, var);
      if (E_OK == ret) {
        if (var->offset >= var->length) {
          SQP_CRM_AND_FREE(TxTpMsg);
          (void)SomeIp_WaitResponse(config, var->methodId, var->sessionId);
        }
      } else { /* abort this tx */
        SQP_CRM_AND_FREE(TxTpMsg);
      }
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainClientWaitResMsg(const SomeIp_ClientServiceType *config) {
  SomeIp_ClientServiceContextType *context = config->context;
  DEC_SQP(WaitResMsg);
  const SomeIp_ClientMethodType *method = NULL;
  uint32_t requestId;

  SQP_WHILE(WaitResMsg) {
    method = &config->methods[var->methodId];
    if (var->timer > 0) {
      var->timer--;
    }
    if (0 == var->timer) {
      requestId = ((uint32_t)var->methodId << 16) + var->sessionId;
      method->onError(requestId, SOMEIPXF_E_TIMEOUT);
      SQP_CRM_AND_FREE(WaitResMsg);
    }
  }
  SQP_WHILE_END()
}

static void SomeIp_MainClient(const SomeIp_ClientServiceType *config) {
  SomeIp_MainClientTxTpMsg(config);
  SomeIp_MainClientRxTpMsg(config);
  SomeIp_MainClientRxTpEvtMsg(config);
  SomeIp_MainClientWaitResMsg(config);
}

void SomeIp_MainFunction(void) {
  int i;
  //printf("SomeIp Mainfunc\n");
  for (i = 0; i < SOMEIP_CONFIG->numOfService; i++) {
    if (SOMEIP_CONFIG->services[i].isServer) {
      SomeIp_MainServer((const SomeIp_ServerServiceType *)SOMEIP_CONFIG->services[i].service);
    } else {
      SomeIp_MainClient((const SomeIp_ClientServiceType *)SOMEIP_CONFIG->services[i].service);
    }
  }
}

static Std_ReturnType SomeIp_DecodeHeader(const uint8_t *data, uint32_t length,
                                          SomeIp_HeaderType *header) {
  Std_ReturnType ret = E_OK;
  if (length >= 16) {
    if (data[12] != 1) { /* @SWS_SomeIpXf_00029 */
      printf("invlaid protocol version\n");
      ret = SOMEIPXF_E_WRONG_PROTOCOL_VERSION;
    } else {
      header->length =
        ((uint32_t)data[4] << 24) + ((uint32_t)data[5] << 16) + ((uint32_t)data[6] << 8) + data[7];
      header->serviceId = ((uint16_t)data[0] << 8) + data[1];
      header->methodId = ((uint16_t)data[2] << 8) + data[3];
      header->clientId = ((uint16_t)data[8] << 8) + data[9];
      header->sessionId = ((uint16_t)data[10] << 8) + data[11];
      header->interfaceVersion = data[13];
      header->messageType = data[14];
      header->returnCode = data[15];
      if (header->messageType & SOMEIP_TP_FLAG) {
        header->messageType &= ~SOMEIP_TP_FLAG;
        header->isTpFlag = TRUE;
        if ((header->length > (8 + 4 + SOMEIP_TP_MAX)) || (header->length <= (8 + 4))) {
          printf("TP length not valid\n");
          ret = SOMEIPXF_E_MALFORMED_MESSAGE;
        }
      } else {
        header->isTpFlag = FALSE;
      }
      if (header->length < 8) {
        ret = E_NOT_OK;
        printf("invalid length\n");
      } else if ((header->length + 8) > length) {
        ret = SOMEIP_E_MSG_TOO_SHORT;
      } else if ((header->length + 8) < length) {
        ret = SOMEIP_E_MSG_TOO_LARGE;
      } else {
      }
    }
  } else {
    printf("message too short\n");
    ret = SOMEIPXF_E_MALFORMED_MESSAGE;
  }
  return ret;
}

static Std_ReturnType SomeIp_DecodeMsg(const PduInfoType *PduInfoPtr, SomeIp_MsgType *msg) {
  Std_ReturnType ret;
  uint32_t offset;

  ret = SomeIp_DecodeHeader(PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength, &msg->header);
  if (E_OK == ret) {
    msg->RemoteAddr = *(const TcpIp_SockAddrType *)PduInfoPtr->MetaDataPtr;
    if (msg->header.isTpFlag) {
      offset = ((uint32_t)PduInfoPtr->SduDataPtr[16] << 24) +
               ((uint32_t)PduInfoPtr->SduDataPtr[17] << 16) +
               ((uint32_t)PduInfoPtr->SduDataPtr[18] << 8) + PduInfoPtr->SduDataPtr[19];
      msg->tpHeader.offset = offset & 0xFFFFFFF0UL;
      msg->tpHeader.moreSegmentsFlag = offset & 0x01;
      msg->req.data = &PduInfoPtr->SduDataPtr[20];
      msg->req.length = PduInfoPtr->SduLength - 20;
    } else {
      msg->req.data = &PduInfoPtr->SduDataPtr[16];
      msg->req.length = PduInfoPtr->SduLength - 16;
    }
  }

  return ret;
}

static Std_ReturnType
SomeIp_ProcessRxTpMsg(uint16_t conId, SomeIp_RxTpMsgList *pendingRxTpMsgs, uint16_t methodId,
                      SomeIp_OnTpCopyRxDataFncType onTpCopyRxData, SomeIp_MsgType *msg)

{
  Std_ReturnType ret = E_OK;
  SomeIp_TpMessageType tpMsg;
  SomeIp_RxTpMsgType *var = NULL;
  uint32_t requestId;
  if (NULL != onTpCopyRxData) {
    var = SomeIp_RxTpMsgFind(pendingRxTpMsgs, methodId);
    if (NULL == var) {
      if ((0 == msg->tpHeader.offset) && (msg->tpHeader.moreSegmentsFlag)) {
        printf("%x:%x:%x:%d FF lenght = %d\n", msg->header.serviceId, msg->header.methodId,
                       msg->header.clientId, msg->header.sessionId, msg->req.length);
        SQP_ALLOC(RxTpMsg);
        if (NULL == var) {
          ret = SOMEIP_E_NOMEM;
          printf("%x:%x:%x:%d OoM for Tp Rx\n", msg->header.serviceId,
                          msg->header.methodId, msg->header.clientId, msg->header.sessionId);
        } else {
          var->offset = 0;
          var->clientId = msg->header.clientId;
          var->sessionId = msg->header.sessionId;
          var->RemoteAddr = msg->RemoteAddr;
          var->methodId = methodId;
          var->timer = SOMEIP_CONFIG->TpRxTimeoutTime;
          SQP_LAPPEND(RxTpMsg);
        }
      } else {
        ret = SOMEIPXF_E_MALFORMED_MESSAGE;
        printf("%x:%x:%x:%d Tp message malformed or loss\n", msg->header.serviceId,
                        msg->header.methodId, msg->header.clientId, msg->header.sessionId);
      }
    } else {
      if ((var->clientId == msg->header.clientId) && (var->sessionId == msg->header.sessionId) &&
          (var->offset == msg->tpHeader.offset) &&
          (0 == memcmp(&var->RemoteAddr, &msg->RemoteAddr, sizeof(TcpIp_SockAddrType)))) {
        var->timer = SOMEIP_CONFIG->TpRxTimeoutTime;
        printf("%x:%x:%x:%d %s lenght = %d, offset = %d\n", msg->header.serviceId,
                       msg->header.methodId, msg->header.clientId, msg->header.sessionId,
                       msg->tpHeader.moreSegmentsFlag ? "CF" : "LF", msg->req.length, var->offset);
      } else {
        SQP_LRM_AND_FREE(RxTpMsg);
        ret = SOMEIPXF_E_MALFORMED_MESSAGE;
        printf("%x:%x:%x:%d Tp message not as expected, loss maybe\n", msg->header.serviceId,
               msg->header.methodId, msg->header.clientId, msg->header.sessionId);
      }
    }
  } else {
    ret = SOMEIPXF_E_MALFORMED_MESSAGE;
  }

  if (E_OK == ret) {
    tpMsg.data = msg->req.data;
    tpMsg.length = msg->req.length;
    tpMsg.offset = msg->tpHeader.offset;
    tpMsg.moreSegmentsFlag = msg->tpHeader.moreSegmentsFlag;
    requestId = ((uint32_t)msg->header.clientId << 16) + msg->header.sessionId;
    ret = onTpCopyRxData(requestId, &tpMsg);
    if (E_OK == ret) {
      var->offset += msg->req.length;
      if (msg->tpHeader.moreSegmentsFlag) {
        ret = SOMEIP_E_OK_SILENT;
      } else {
        msg->req.data = tpMsg.data;
        msg->req.length = var->offset;
        SQP_LRM_AND_FREE(RxTpMsg);
      }
    }
  }

  return ret;
}

static Std_ReturnType SomeIp_HandleServerMessage_Request(const SomeIp_ServerServiceType *config,
                                                         uint16_t conId, SomeIp_MsgType *msg) {
  Std_ReturnType ret = SOMEIPXF_E_UNKNOWN_METHOD;
  uint16_t methodId;
  const SomeIp_ServerConnectionType *connection = &config->connections[conId];
  SomeIp_ServerConnectionContextType *context = connection->context;
  const SomeIp_ServerMethodType *method = NULL;
  uint32_t requestId;

  printf("SomeIp_HandleServerMessage_Request\n");
  for (methodId = 0; methodId < config->numOfMethods; methodId++) {
    if (config->methods[methodId].methodId == msg->header.methodId) {
      if (((0xFF == config->methods[methodId].interfaceVersion) ||
           (config->methods[methodId].interfaceVersion == msg->header.interfaceVersion))) {
        method = &config->methods[methodId];
        ret = E_OK;
        break;
      } else {
        ret = SOMEIPXF_E_WRONG_INTERFACE_VERSION;
      }
    }
  }

  if (E_OK == ret) {
    if (msg->header.isTpFlag) {
      ret = SomeIp_ProcessRxTpMsg(conId, &context->pendingRxTpMsgs, methodId,
                                  method->onTpCopyRxData, msg);
    }
  }

  if (E_OK == ret) {
    requestId = ((uint32_t)msg->header.clientId << 16) + msg->header.sessionId;
    if (SOMEIP_MSG_REQUEST == msg->header.messageType) {
      ret = SomeIp_ProcessRequest(config, conId, methodId, msg);
    } else {
      ret = method->onFireForgot(requestId, &msg->req);
      if (E_OK == ret) {
        ret = SOMEIP_E_OK_SILENT;
      }
    }
  }

  return ret;
}

static Std_ReturnType SomeIp_HandleServerMessage(const SomeIp_ServerServiceType *config,
                                                 uint16_t conId, SomeIp_MsgType *msg) {
  Std_ReturnType ret = E_OK;
  if (conId < config->numOfConnections) {
    if (config->serviceId != msg->header.serviceId) {
      printf("config->serviceId:%x msg->header.serviceId:%x\n", config->serviceId, msg->header.serviceId);
      printf("unknown service\n");
      ret = SOMEIPXF_E_UNKNOWN_SERVICE;
    }
  } else {
    printf("invalid connection ID\n");
    ret = E_NOT_OK;
  }

  if (E_OK == ret) {
    switch (msg->header.messageType) {
    case SOMEIP_MSG_REQUEST:
    case SOMEIP_MSG_REQUEST_NO_RETURN:
      ret = SomeIp_HandleServerMessage_Request(config, conId, msg);
      break;
    case SOMEIP_MSG_NOTIFICATION:
      /* slient as event multicast */
      break;
    default:
      printf("server: unsupported message type 0x%x\n", msg->header.messageType);
      ret = SOMEIPXF_E_WRONG_MESSAGE_TYPE;
      break;
    }
  }

  if (SOMEIP_E_PENDING == ret) {
    /* ok silent */
  } else if (E_OK != ret) {
    (void)SomeIp_ReplyError(config->connections[conId].TxPduId, msg, ret);
  } else {
    /* ok, pass */
  }

  return ret;
}

static Std_ReturnType
SomeIp_HandleClientMessage_Notification(const SomeIp_ClientServiceType *config,
                                        SomeIp_MsgType *msg) {
  Std_ReturnType ret = E_NOT_OK;
  SomeIp_ClientServiceContextType *context = config->context;
  const SomeIp_ClientEventType *event = NULL;
  uint16_t eventId;
  uint32_t requestId;

  for (eventId = 0; eventId < config->numOfEvents; eventId++) {
    if (config->events[eventId].eventId == msg->header.methodId) {
      if (((0xFF == config->events[eventId].interfaceVersion) ||
           (config->events[eventId].interfaceVersion == msg->header.interfaceVersion))) {
        event = &config->events[eventId];
        ret = E_OK;
        break;
      } else {
        ret = SOMEIPXF_E_WRONG_INTERFACE_VERSION;
      }
    }
  }

  if (E_OK == ret) {
    if (msg->header.isTpFlag) {
      ret =
        SomeIp_ProcessRxTpMsg(0, &context->pendingRxTpEvtMsgs, eventId, event->onTpCopyRxData, msg);
    }
  }

  if (E_OK == ret) {
    requestId = ((uint32_t)msg->header.clientId << 16) + msg->header.sessionId;
    ret = event->onNotification(requestId, &msg->req);
  } else {
    ret = SOMEIPXF_E_UNKNOWN_METHOD;
  }

  return ret;
}

static Std_ReturnType SomeIp_HandleClientMessage_Response(const SomeIp_ClientServiceType *config,
                                                         SomeIp_MsgType *msg) {
  Std_ReturnType ret = SOMEIPXF_E_UNKNOWN_METHOD;
  const SomeIp_ClientMethodType *method = NULL;
  SomeIp_ClientServiceContextType *context = config->context;
  uint16_t methodId;
  uint32_t requestId;

  for (methodId = 0; methodId < config->numOfMethods; methodId++) {
    if (config->methods[methodId].methodId == msg->header.methodId) {
      if (((0xFF == config->methods[methodId].interfaceVersion) ||
           (config->methods[methodId].interfaceVersion == msg->header.interfaceVersion))) {
        method = &config->methods[methodId];
        ret = E_OK;
        break;
      } else {
        ret = SOMEIPXF_E_WRONG_INTERFACE_VERSION;
      }
    }
  }

  if (E_OK == ret) {
    ret = SomeIp_IsResponseExpected(config, methodId, msg);
    if (E_OK != ret) {
      printf("response %x:%x:%d not as expected\n", config->serviceId, method->methodId,
                      msg->header.sessionId);
    }
  }

  if (E_OK == ret) {
    if (msg->header.isTpFlag) {
      ret =
        SomeIp_ProcessRxTpMsg(0, &context->pendingRxTpMsgs, methodId, method->onTpCopyRxData, msg);
    }
  }

  if (E_OK == ret) {
    requestId = ((uint32_t)msg->header.clientId << 16) + msg->header.sessionId;
    if (E_OK == msg->header.returnCode) {
      ret = method->onResponse(requestId, &msg->req);
    } else {
      ret = method->onError(requestId, msg->header.returnCode);
    }
  }

  return ret;
}

static Std_ReturnType SomeIp_HandleClientMessage(const SomeIp_ClientServiceType *config,
                                                 SomeIp_MsgType *msg) {
  Std_ReturnType ret = E_OK;
  //printf("SomeIp_HandleClientMessage!!!!!!!!!!!!!!!!!!!!!!!\\n");
  if (config->serviceId != msg->header.serviceId) {
    //printf("config->serviceId:%x msg->header.serviceId:%x\n", config->serviceId, msg->header.serviceId);
    //printf("unknown service\n");
    ret = SOMEIPXF_E_UNKNOWN_SERVICE;
  }

  if (E_OK == ret) {
    switch (msg->header.messageType) {
    case SOMEIP_MSG_NOTIFICATION:
      ret = SomeIp_HandleClientMessage_Notification(config, msg);
      break;
    case SOMEIP_MSG_RESPONSE:
      ret = SomeIp_HandleClientMessage_Response(config, msg);
      break;
    case SOMEIP_MSG_ERROR:
      ret = SomeIp_HandleClientMessage_Response(config, msg);
      break;
    default:
      printf("client: unsupported message type 0x%x\n", msg->header.messageType);
      ret = SOMEIPXF_E_WRONG_MESSAGE_TYPE;
      break;
    }
  }

  return ret;
}

static Std_ReturnType SomeIp_HandleRxMsg(PduIdType RxPduId, SomeIp_MsgType *msg) {
  Std_ReturnType ret;
  uint16_t index;
  printf("[%d] service 0x%x:0x%x:%d message type %d return code %d payload %d bytes "
                 "from client 0x%x:%d %d.%d.%d.%d:%d\n",
                 RxPduId, msg->header.serviceId, msg->header.methodId, msg->header.interfaceVersion,
                 msg->header.messageType, msg->header.returnCode, msg->header.length - 8,
                 msg->header.clientId, msg->header.sessionId, msg->RemoteAddr.addr[0],
                 msg->RemoteAddr.addr[1], msg->RemoteAddr.addr[2], msg->RemoteAddr.addr[3],
                 msg->RemoteAddr.port);
  if (RxPduId < SOMEIP_CONFIG->numOfPIDs) {
    index = SOMEIP_CONFIG->PID2ServiceMap[RxPduId];
    if (index < SOMEIP_CONFIG->numOfService) {
      if (SOMEIP_CONFIG->services[index].isServer) {
        ret = SomeIp_HandleServerMessage(
          (const SomeIp_ServerServiceType *)SOMEIP_CONFIG->services[index].service,
          SOMEIP_CONFIG->PID2ServiceConnectionMap[RxPduId], msg);
      } else {
        ret = SomeIp_HandleClientMessage(
          (const SomeIp_ClientServiceType *)SOMEIP_CONFIG->services[index].service, msg);
      }
    } else {
      ret = E_NOT_OK;
      printf("invalid configuration for PID2ServiceMap\n");
    }
  } else {
    ret = SOMEIPXF_E_NOT_REACHABLE;
  }

  return ret;
}

static void SomeIp_ServerServiceModeChg(const SomeIp_ServerServiceType *service, uint16_t conId,
                                        SoAd_SoConModeType Mode) {
  SomeIp_ServerConnectionContextType *context = service->connections[conId].context;
  int i;
  printf("SomeIp_ServerServiceModeChg\n");
  if (SOAD_SOCON_OFFLINE == Mode) {
    SQP_CLEAR(AsyncReqMsg);
    SQP_CLEAR(RxTpMsg);
    SQP_CLEAR(TxTpMsg);
    SQP_CLEAR(TxTpEvtMsg);
    context->online = FALSE;
    service->onConnect(conId, FALSE);
    for (i = 0; i < service->numOfEvents; i++) {
      Sd_RemoveSubscriber(service->events[i].sdHandleID, service->connections[conId].TxPduId);
    }
  } else {
    context->online = TRUE;
    service->onConnect(conId, TRUE);
  }
}

static void SomeIp_ClientServiceModeChg(const SomeIp_ClientServiceType *service,
                                        SoAd_SoConModeType Mode) {
  SomeIp_ClientServiceContextType *context = service->context;
  if (SOAD_SOCON_OFFLINE == Mode) {
    //printf()
    SQP_CLEAR(RxTpEvtMsg);
    SQP_CLEAR(RxTpMsg);
    SQP_CLEAR(TxTpMsg);
    context->online = FALSE;
    service->onAvailability(FALSE);
  } else {
    context->online = TRUE;
    service->onAvailability(TRUE);
  }
}

void SomeIp_SoConModeChg(SoAd_SoConIdType SoConId, SoAd_SoConModeType Mode) {
  Std_ReturnType ret = E_NOT_OK;
  const SomeIp_ServiceType *service = NULL;
  const SomeIp_ServerServiceType *ss = NULL;
  SomeIp_ServerContextType *context;
  uint16_t i, j;
  uint16_t conId = 0;

  //printf("SoConId %d Mode %d\n", SoConId, Mode);
  for (i = 0; (i < SOMEIP_CONFIG->numOfService) && (E_NOT_OK == ret); i++) {
    service = &SOMEIP_CONFIG->services[i];
    if (service->SoConId == SoConId) {
      ret = E_OK;
    } else if (service->isServer) {
      ss = (const SomeIp_ServerServiceType *)service->service;
      for (j = 0; (j < ss->numOfConnections) && (E_NOT_OK == ret); j++) {
        if (ss->connections[j].SoConId == SoConId) {
          conId = j;
          ret = E_OK;
        }
      }
    } else {
      /* not match */
    }
  }
  if (E_OK == ret) {
    if (service->isServer) {
      if (service->SoConId == SoConId) {
        ss = (const SomeIp_ServerServiceType *)service->service;
        context = ss->context;
        if (SOAD_SOCON_OFFLINE == Mode) {
          SQP_CLEAR(TxTpEvtMsg);
          context->online = FALSE;
        } else {
          context->online = TRUE;
        }
        if (TCPIP_IPPROTO_TCP == ss->protocol) {
          if (SOAD_SOCON_OFFLINE == Mode) {
            for (j = 0; j < ss->numOfConnections; j++) {
              (void)SoAd_CloseSoCon(ss->connections[j].SoConId, TRUE);
            }
          }
        } else {
          SomeIp_ServerServiceModeChg(ss, 0, Mode);
        }
      } else {
        SomeIp_ServerServiceModeChg((const SomeIp_ServerServiceType *)service->service, conId,
                                    Mode);
      }
    } else {
      SomeIp_ClientServiceModeChg((const SomeIp_ClientServiceType *)service->service, Mode);
    }
  } else {
    //printf("SoConId %d unknown\n", SoConId);
  }
}

void SomeIp_task(void *pvParameters)
{
  for (;;)
  {

    SomeIp_MainFunction();
    vTaskDelay(1);

  } // for
}

void SomeIp_RxIndication(PduIdType RxPduId, const PduInfoType *PduInfoPtr) {
  SomeIp_MsgType msg;
  Std_ReturnType ret;

  printf("SomeIp msg received!\n");
  ret = SomeIp_DecodeMsg(PduInfoPtr, &msg);
  if (E_OK == ret) {
    SomeIp_HandleRxMsg(RxPduId, &msg);
  } else {
    printf("IF message malformed\n");
  }
}