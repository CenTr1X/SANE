#ifndef _SOMEIP_H
#define _SOMEIP_H

#include "SomeIpSd.h"

/* ================================ [ MACROS    ] ============================================== */
#define SOMEIP_E_PENDING 100
#define SOMEIP_E_OK_SILENT SOMEIP_E_PENDING
#define SOMEIP_E_NOMEM 101
#define SOMEIP_E_MSG_TOO_SHORT 102
#define SOMEIP_E_MSG_TOO_LARGE 103
#define SOMEIP_E_BUSY 104

/* @SWS_SomeIpXf_00168 @SWS_SomeIpXf_00115 @PRS_SOMEIP_00191 */
#define SOMEIPXF_E_UNKNOWN_SERVICE 0x02
#define SOMEIPXF_E_UNKNOWN_METHOD 0x03
#define SOMEIPXF_E_NOT_READY 0x04
#define SOMEIPXF_E_NOT_REACHABLE 0x05
#define SOMEIPXF_E_TIMEOUT 0x06
#define SOMEIPXF_E_WRONG_PROTOCOL_VERSION 0x07
#define SOMEIPXF_E_WRONG_INTERFACE_VERSION 0x08
#define SOMEIPXF_E_MALFORMED_MESSAGE 0x09
#define SOMEIPXF_E_WRONG_MESSAGE_TYPE 0x0a
/* ================================ [ TYPES     ] ============================================== */
/* @SWS_SomeIpXf_00152 */
typedef struct {
  uint16_t serviceId;
  uint16_t methodId;
  uint32_t length;
  uint16_t clientId;
  uint16_t sessionId;
  uint8_t interfaceVersion;
  uint8_t messageType;
  uint8_t returnCode;
  boolean isTpFlag;
} SomeIp_HeaderType;

typedef struct {
  uint32_t offset;
  boolean moreSegmentsFlag;
} SomeIp_TpHeaderType;

typedef struct {
  uint8_t *data;
  uint32_t offset;
  uint32_t length;
  boolean moreSegmentsFlag;
} SomeIp_TpMessageType;

typedef struct {
  uint8_t *data;
  uint32_t length;
} SomeIp_MessageType; //raw message

typedef struct {
  SomeIp_HeaderType header;
  SomeIp_TpHeaderType tpHeader;
  TcpIp_SockAddrType RemoteAddr;
  SomeIp_MessageType req;
} SomeIp_MsgType; //message with metadata

/* API for service */
typedef void (*SomeIp_OnAvailabilityFncType)(boolean isAvailable);
typedef void (*SomeIp_OnConnectFncType)(uint16_t condId, boolean isConnected);

/* API for method */
typedef Std_ReturnType (*SomeIp_OnRequestFncType)(uint32_t requestId, SomeIp_MessageType *req,
                                                  SomeIp_MessageType *res);
typedef Std_ReturnType (*SomeIp_OnFireForgotFncType)(uint32_t requestId, SomeIp_MessageType *req);
typedef Std_ReturnType (*SomeIp_OnAsyncRequestFncType)(uint32_t requestId, SomeIp_MessageType *res);

/* For the LF, set the msg->data as beginning of the buffer */
/* msg == NULL to indicate transmission error, abort current transmit */
typedef Std_ReturnType (*SomeIp_OnTpCopyRxDataFncType)(uint32_t requestId,
                                                       SomeIp_TpMessageType *msg);
typedef Std_ReturnType (*SomeIp_OnTpCopyTxDataFncType)(uint32_t requestId,
                                                       SomeIp_TpMessageType *msg);

typedef Std_ReturnType (*SomeIp_OnResponseFncType)(uint32_t requestId, SomeIp_MessageType *res);
typedef Std_ReturnType (*SomeIp_OnErrorFncType)(uint32_t requestId, Std_ReturnType ercd);

/* API for events */
typedef Std_ReturnType (*SomeIp_OnNotificationFncType)(uint32_t requestId, SomeIp_MessageType *evt);

typedef struct {
  boolean isServer;
  SoAd_SoConIdType SoConId;
  const void *service;
} SomeIp_ServiceType;

typedef struct {
  uint16_t methodId;
  uint8_t interfaceVersion;
  SomeIp_OnRequestFncType onRequest;
  SomeIp_OnFireForgotFncType onFireForgot;
  SomeIp_OnAsyncRequestFncType onAsyncRequest;
  SomeIp_OnTpCopyRxDataFncType onTpCopyRxData;
  SomeIp_OnTpCopyTxDataFncType onTpCopyTxData;
  uint32_t resMaxLen;
} SomeIp_ServerMethodType;

typedef struct {
  uint16_t sdHandleID;
  uint16_t eventId;
  uint8_t interfaceVersion;
  SomeIp_OnTpCopyTxDataFncType onTpCopyTxData;
} SomeIp_ServerEventType;

typedef struct {
  uint16_t methodId;
  uint8_t interfaceVersion;
  SomeIp_OnResponseFncType onResponse;
  SomeIp_OnErrorFncType onError;
  SomeIp_OnTpCopyRxDataFncType onTpCopyRxData;
  SomeIp_OnTpCopyTxDataFncType onTpCopyTxData;
} SomeIp_ClientMethodType;

typedef struct {
  uint16_t eventId;
  uint8_t interfaceVersion;
  SomeIp_OnNotificationFncType onNotification;
  SomeIp_OnTpCopyRxDataFncType onTpCopyRxData;
} SomeIp_ClientEventType;

typedef STAILQ_HEAD(rxTpMsgHead, SomeIp_RxTpMsg_s) SomeIp_RxTpMsgList;
typedef STAILQ_HEAD(txTpMsgHead, SomeIp_TxTpMsg_s) SomeIp_TxTpMsgList;
typedef STAILQ_HEAD(txTpEvtMsgHead, SomeIp_TxTpEvtMsg_s) SomeIp_TxTpEvtMsgList;
typedef STAILQ_HEAD(txWaitResMsgHead, SomeIp_WaitResMsg_s) SomeIp_WaitResMsgList;

typedef struct {
  STAILQ_HEAD(reqMsgHead, SomeIp_AsyncReqMsg_s) pendingAsyncReqMsgs;
  SomeIp_RxTpMsgList pendingRxTpMsgs;
  SomeIp_TxTpMsgList pendingTxTpMsgs;
  SomeIp_TxTpEvtMsgList pendingTxTpEvtMsgs;
  bool online;
  bool takenControled;
} SomeIp_ServerConnectionContextType;

/* For TCP large messages */
typedef struct {
  uint8_t *data;
  uint32_t length;
  uint32_t offset;
  uint8_t header[16];
  uint8_t lenOfHd;
} SomeIp_TcpBufferType;

typedef struct {
  SomeIp_ServerConnectionContextType *context;
  PduIdType TxPduId;
  SoAd_SoConIdType SoConId;
  SomeIp_TcpBufferType *tcpBuf;
} SomeIp_ServerConnectionType;

typedef struct {
  SomeIp_TxTpEvtMsgList pendingTxTpEvtMsgs;
  bool online;
} SomeIp_ServerContextType;

typedef struct {
  uint16_t serviceId;
  uint16_t clientId;
  const SomeIp_ServerMethodType *methods;
  uint16_t numOfMethods;
  const SomeIp_ServerEventType *events;
  uint16_t numOfEvents;
  const SomeIp_ServerConnectionType *connections;
  uint8_t numOfConnections;
  TcpIp_ProtocolType protocol;
  SomeIp_ServerContextType *context;
  uint16_t SeparationTime; /* @ECUC_SomeIpTp_00006 */
  SomeIp_OnConnectFncType onConnect;
} SomeIp_ServerServiceType;

typedef struct {
  SomeIp_RxTpMsgList pendingRxTpEvtMsgs;
  SomeIp_RxTpMsgList pendingRxTpMsgs;
  SomeIp_TxTpMsgList pendingTxTpMsgs;
  SomeIp_WaitResMsgList pendingWaitResMsgs;
  bool online;
} SomeIp_ClientServiceContextType;

typedef struct {
  uint16_t serviceId;
  uint16_t clientId;
  uint16_t sdHandleID;
  const SomeIp_ClientMethodType *methods;
  uint16_t numOfMethods;
  const SomeIp_ClientEventType *events;
  uint16_t numOfEvents;
  SomeIp_ClientServiceContextType *context;
  PduIdType TxPduId;
  SomeIp_OnAvailabilityFncType onAvailability;
  SomeIp_TcpBufferType *tcpBuf;
  uint16_t SeparationTime; /* @ECUC_SomeIpTp_00006 */
  uint16_t ResponseTimeout;
} SomeIp_ClientServiceType;

typedef struct SomeIp_Config_s {
  /* @ECUC_SomeIpTp_00023 */
  uint16_t TpRxTimeoutTime;
  const SomeIp_ServiceType *services;
  uint16_t numOfService;
  const uint16_t *PID2ServiceMap;
  const uint16_t *PID2ServiceConnectionMap;
  uint16_t numOfPIDs;
  const uint16_t *TxMethod2ServiceMap;
  const uint16_t *TxMethod2PerServiceMap;
  uint16_t numOfTxMethods;
  const uint16_t *TxEvent2ServiceMap;
  const uint16_t *TxEvent2PerServiceMap;
  uint16_t numOfTxEvents;
} SomeIp_ConfigType; 

typedef struct SomeIp_AsyncReqMsg_s {
  STAILQ_ENTRY(SomeIp_AsyncReqMsg_s) entry;
  TcpIp_SockAddrType RemoteAddr;
  uint16_t clientId;
  uint16_t sessionId;
  uint16_t methodId;
} SomeIp_AsyncReqMsgType;

typedef struct SomeIp_RxTpMsg_s {
  STAILQ_ENTRY(SomeIp_RxTpMsg_s) entry;
  TcpIp_SockAddrType RemoteAddr;
  uint32_t offset;
  uint16_t methodId; /* this is the key */
  uint16_t clientId;
  uint16_t sessionId;
  uint16_t timer;
} SomeIp_RxTpMsgType;

#define SomeIp_RxTpEvtMsg_s SomeIp_RxTpMsg_s
typedef SomeIp_RxTpMsgType SomeIp_RxTpEvtMsgType;

typedef struct SomeIp_TxTpMsg_s {
  STAILQ_ENTRY(SomeIp_TxTpMsg_s) entry;
  TcpIp_SockAddrType RemoteAddr;
  uint32_t offset;
  uint32_t length;
  uint16_t methodId; /* this is the key */
  uint16_t clientId;
  uint16_t sessionId;
  uint16_t timer;
  uint8_t messageType;
#ifndef DISABLE_SOMEIP_TX_NOK_RETRY
  uint8_t retryCounter;
#endif
} SomeIp_TxTpMsgType;

typedef struct SomeIp_TxTpEvtMsg_s {
  STAILQ_ENTRY(SomeIp_TxTpEvtMsg_s) entry;
  uint32_t offset;
  uint32_t length;
  uint16_t eventId; /* this is the key */
  uint16_t sessionId;
  uint16_t timer;
} SomeIp_TxTpEvtMsgType;

typedef struct SomeIp_WaitResMsg_s {
  STAILQ_ENTRY(SomeIp_WaitResMsg_s) entry;
  uint16_t methodId;
  uint16_t sessionId;
  uint16_t timer;
} SomeIp_WaitResMsgType;

Std_ReturnType SomeIp_ResolveSubscriber(uint16_t ServiceId, Sd_EventHandlerSubscriberType *sub);

void onSubscribe_common(uint16_t eventGroupId, TcpIp_SockAddrType* RemoteAddr);

static Std_ReturnType SomeIp_Transmit(PduIdType TxPduId, TcpIp_SockAddrType *RemoteAddr,
                                      uint8_t *data, uint16_t serviceId, uint16_t methodId,
                                      uint16_t clientId, uint16_t sessionId,
                                      uint8_t interfaceVersion, uint8_t messageType,
                                      uint8_t returnCode, uint32_t payloadLength);

void SomeIp_task(void *pvParameters);


Std_ReturnType SomeIp_Request(uint32_t requestId, uint8_t *data, uint32_t length);
Std_ReturnType SomeIp_FireForgot(uint32_t requestId, uint8_t *data, uint32_t length);
Std_ReturnType SomeIp_Notification(uint32_t requestId, uint8_t *data, uint32_t length);
static Std_ReturnType SomeIp_TransmitEvtMsg(const SomeIp_ServerServiceType *config,
                                            const SomeIp_ServerEventType *event, uint8_t *data,
                                            uint32_t payloadLength, bool isTp,
                                            Sd_EventHandlerSubscriberListType *list,
                                            uint16_t sessionId);

void SomeIp_RxIndication(PduIdType RxPduId, const PduInfoType *PduInfoPtr);
void SomeIp_SoConModeChg(SoAd_SoConIdType SoConId, SoAd_SoConModeType Mode);
void SomeIp_Init(const SomeIp_ConfigType *ConfigPtr);
#endif