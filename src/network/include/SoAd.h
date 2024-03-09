#ifndef _SOAD_H
#define _SOAD_H
/* ================================ [ INCLUDES  ]
 * ============================================== */
#include "ComStack_Types.h"
#include "TcpIp.h"

/* ================================ [ MACROS    ] ============================================== */
#define SOAD_SOCON_TCP_SERVER ((SoAd_SoConTypeType)0x01)
#define SOAD_SOCON_TCP_CLIENT ((SoAd_SoConTypeType)0x02)
#define SOAD_SOCON_TCP_ACCEPT ((SoAd_SoConTypeType)0x04)
#define SOAD_SOCON_UDP_SERVER ((SoAd_SoConTypeType)0x08)
#define SOAD_SOCON_UDP_CLIENT ((SoAd_SoConTypeType)0x10)
/* ================================ [ TYPES     ]
 * ============================================== */
typedef enum {
  SOAD_SOCON_ONLINE,
  SOAD_SOCON_RECONNECT,
  SOAD_SOCON_OFFLINE
} SoAd_SoConModeType;

typedef enum {
  SOAD_SOCKET_CLOSED,
  SOAD_SOCKET_CREATE,
  SOAD_SOCKET_ACCEPT,
  SOAD_SOCKET_READY,
  SOAD_SOCKET_TAKEN_CONTROL,
} SoAd_SocketStateType;

typedef uint8_t SoAd_SoConTypeType;

/* @SWS_SoAd_00518 */
typedef uint16_t SoAd_SoConIdType;

/* @ECUC_SoAd_00009 */
// Specifies the socket connection
typedef struct {
  PduIdType RxPduId;
  SoAd_SoConIdType SoConId;
  uint16_t GID;
  SoAd_SoConTypeType SoConType;
} SoAd_SocketConnectionType;

typedef struct {
  TcpIp_SocketIdType sock;
  SoAd_SocketStateType state;
  TcpIp_SockAddrType RemoteAddr;
  TcpIp_SockAddrType LocalAddr;
#if SOAD_ERROR_COUNTER_LIMIT > 0
  uint8_t errorCounter;
#endif
} SoAd_SocketContextType;


/***Function Pointers ***/
typedef void (*SoAd_SoConModeChgNotificationFncType)(SoAd_SoConIdType SoConId,
                                                     SoAd_SoConModeType Mode);

/* @SWS_SoAd_00663 */
typedef Std_ReturnType (*SoAd_IfTriggerTransmitFncType)(PduIdType TxPduId, PduInfoType *PduInfoPtr);

/* @SWS_SoAd_00106 */
typedef void (*SoAd_IfRxIndicationFncType)(PduIdType RxPduId, const PduInfoType *PduInfoPtr);

/* @SWS_SoAd_00107 */
typedef void (*SoAd_IfTxConfirmationFncType)(PduIdType id, Std_ReturnType result);

/* @SWS_SoAd_00138 */
typedef BufReq_ReturnType (*SoAd_TpStartOfReceptionFncType)(PduIdType id, const PduInfoType *info,
                                                            PduLengthType TpSduLength,
                                                            PduLengthType *bufferSizePtr);

/* @SWS_SoAd_00139 */
typedef BufReq_ReturnType (*SoAd_TpCopyRxDataFncType)(PduIdType id, const PduInfoType *info,
                                                      PduLengthType *bufferSizePtr);

/* @SWS_SoAd_00180 */
typedef void (*SoAd_TpRxIndicationFncType)(PduIdType id, Std_ReturnType result);

/* @SWS_SoAd_00137 */
typedef BufReq_ReturnType (*SoAd_TpCopyTxDataFncType)(PduIdType id, const PduInfoType *info,
                                                      const RetryInfoType *retry,
                                                      PduLengthType *availableDataPtr);

/* @SWS_SoAd_00181 */
typedef void (*SoAd_TpTxConfirmationFncType)(PduIdType id, Std_ReturnType result);
/***---Function Pointers ---***/


/* @ECUC_SoAd_00130 */
typedef struct {
  /* SoAd_IfInterfaceType or SoAd_TpInterfaceType */
  const void *Interface;
  SoAd_SoConModeChgNotificationFncType SoConModeChgNotification;
  TcpIp_ProtocolType ProtocolType;
  /* https://www.ibm.com/docs/en/zvm/6.4?topic=SSB27U_6.4.0/com.ibm.zvm.v640.kiml0/asonetw.htm */
  uint32_t Remote;        /* if not 0, this is the default remote server IPv4 address */
  SoAd_SoConIdType SoConId; /* where the accepted connection socket id start from*/
  uint16_t Port;
  TcpIp_LocalAddrIdType LocalAddrId;
  uint8_t numOfConnections; /* max number of accepted connections */
  boolean AutomaticSoConSetup;
  boolean IsTP;
  boolean IsMulitcast;       /* if True, the Remote is a multicast UDP IPv4 address */
} SoAd_SocketConnectionGroupType;

typedef struct {
  SoAd_IfRxIndicationFncType IfRxIndication;
  SoAd_IfTriggerTransmitFncType IfTriggerTransmit;
  SoAd_IfTxConfirmationFncType IfTxConfirmation;
} SoAd_IfInterfaceType;

typedef struct {
  SoAd_TpStartOfReceptionFncType TpStartOfReception;
  SoAd_TpCopyRxDataFncType TpCopyRxData;
  SoAd_TpRxIndicationFncType TpRxIndication;
  SoAd_TpCopyTxDataFncType TpCopyTxData;
  SoAd_TpTxConfirmationFncType TpTxConfirmation;
} SoAd_TpInterfaceType;

typedef struct SoAd_Config_s {
  const SoAd_SocketConnectionType *Connections;
  SoAd_SocketContextType *Contexts;
  uint16_t numOfConnections;
  const SoAd_SoConIdType *TxPduIdToSoCondIdMap;
  uint8_t numOfTxPduIds;
  const SoAd_SocketConnectionGroupType *ConnectionGroups;
  uint16_t numOfGroups;
} SoAd_ConfigType;

/* ================================ [ FUNCTIONS ]
 * ============================================== */
/* @SWS_SoAd_00093 */
void SoAd_Init(const SoAd_ConfigType *ConfigPtr);

/* @SWS_SoAd_00510 */
//set socket connection from CLOSED to CREATE
Std_ReturnType SoAd_OpenSoCon(SoAd_SoConIdType SoConId);

/* @SWS_SoAd_00511 */
//set socket connection from NOT CLOSED to CLOSED
Std_ReturnType SoAd_CloseSoCon(SoAd_SoConIdType SoConId, boolean abort);

/* @SWS_SoAd_00091 */
Std_ReturnType SoAd_IfTransmit(PduIdType TxPduId, const PduInfoType *PduInfoPtr);

/* @SWS_SoAd_00515 */
Std_ReturnType SoAd_SetRemoteAddr(SoAd_SoConIdType SoConId,
                                  const TcpIp_SockAddrType *RemoteAddrPtr);

/* @SWS_SoAd_00506 */
Std_ReturnType SoAd_GetLocalAddr(SoAd_SoConIdType SoConId, TcpIp_SockAddrType *LocalAddrPtr,
                                 uint8_t *NetmaskPtr, TcpIp_SockAddrType *DefaultRouterPtr);

/* @SWS_SoAd_00509 */
Std_ReturnType SoAd_GetSoConId(PduIdType TxPduId, SoAd_SoConIdType *SoConIdPtr);

void SoAd_MainFunction();
void SoAd_TimerCallback(void* p);
void SoAd_task(void *pvParameters);
void SoAd_TcpEchoServerCallback(PduIdType RxPduId, const PduInfoType *PduInfoPtr);
Std_ReturnType SoAd_GetRemoteAddr(SoAd_SoConIdType SoConId, TcpIp_SockAddrType *IpAddrPtr); 
Std_ReturnType SoAd_TpTransmit(PduIdType TxPduId, const PduInfoType *PduInfoPtr);
#endif 