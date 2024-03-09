#include "DoIP.h"
#include "DoIP_Cfg.h"
#include "SoAd.h"
#include "printf.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "FreeRTOS.h"
#include "task.h"

/* ================================ [ MACROS    ] ============================================== */
#define DOIP_CONFIG (&DoIP_Config)

#define DOIP_PROTOCOL_VERSION 2
#define DOIP_HEADER_LENGTH 8u

#ifndef DOIP_SHORT_MSG_LOCAL_BUFFER_SIZE
#define DOIP_SHORT_MSG_LOCAL_BUFFER_SIZE (DOIP_HEADER_LENGTH + 40)
#endif

/* return this when negative response buffer set */
#define DOIP_E_NOT_OK ((Std_ReturnType)200)

#define DOIP_E_NOT_OK_SILENT ((Std_ReturnType)201)
/* ================================ [ TYPES     ] ============================================== */
typedef struct {
  DoIP_ActivationLineType ActivationLineState;
} DoIP_ContextType;

typedef struct {
  const uint8_t *req;
  uint8_t *res; /* response buffer for this message */
  uint16_t payloadType;
  uint32_t payloadLength;
  PduLengthType reqLen;
  PduLengthType resLen;
} DoIP_MsgType;
/* ================================ [ DECLARES  ] ============================================== */
extern const DoIP_ConfigType DoIP_Config;
/* ================================ [ DATAS     ] ============================================== */
static DoIP_ContextType DoIP_Context;
/* ================================ [ FUNCTIONS ] ============================================== */
void DoIP_Init(const DoIP_ConfigType *ConfigPtr) {
  int i;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  for (i = 0; i < config->numOfUdpVehicleAnnouncementConnections; i++) {
    memset(config->UdpVehicleAnnouncementConnections[i].context, 0,
           sizeof(DoIP_UdpVehicleAnnouncementConnectionContextType));
  }
  for (i = 0; i < config->MaxTesterConnections; i++) {
    memset(config->testerConnections[i].context, 0, sizeof(DoIP_TesterConnectionContextType));
  }
}

static void doipForgetDiagMsg(const DoIP_TesterConnectionType *connection) {
  if (NULL != connection->context->msg.req) {
    /* @SWS_DoIP_00138 */
    free(connection->context->msg.req);
    connection->context->msg.req = NULL;
  }
}

static void doipFillHeader(uint8_t *header, uint16_t payloadType, uint32_t payloadLength) {
  header[0] = DOIP_PROTOCOL_VERSION;
  header[1] = ~DOIP_PROTOCOL_VERSION;
  header[2] = (payloadType >> 8) & 0xFF;
  header[3] = payloadType & 0xFF;
  header[4] = (payloadLength >> 24) & 0xFF;
  header[5] = (payloadLength >> 16) & 0xFF;
  header[6] = (payloadLength >> 8) & 0xFF;
  header[7] = payloadLength & 0xFF;
}


static PduLengthType doipSetupVehicleAnnouncementResponse(uint8_t *data) {
  Std_ReturnType ret;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  doipFillHeader(data, DOIP_VAN_MSG_OR_VIN_RESPONCE, 33);
  /* @SWS_DoIP_00072 */
  ret = config->GetVin(&data[DOIP_HEADER_LENGTH]);
  if (E_OK != ret) {
    memset(&data[DOIP_HEADER_LENGTH], config->VinInvalidityPattern, 17);
    data[DOIP_HEADER_LENGTH + 31] = 0x00; /* Further action byte */
    data[DOIP_HEADER_LENGTH + 32] = 0x10; /* VIN/GID Status */
  } else {
    printf("vin ok\n");
    data[DOIP_HEADER_LENGTH + 31] = 0x00;
    data[DOIP_HEADER_LENGTH + 32] = 0x00;
  }

  data[DOIP_HEADER_LENGTH + 17] = (config->LogicalAddress >> 8) & 0xFF;
  data[DOIP_HEADER_LENGTH + 18] = config->LogicalAddress & 0xFF;

  config->GetEID(&data[DOIP_HEADER_LENGTH + 19]);
  config->GetGID(&data[DOIP_HEADER_LENGTH + 25]);

  return DOIP_HEADER_LENGTH + 33;
}

static void doipHandleInactivityTimer(void) {
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection;
  int i;

  for (i = 0; i < config->MaxTesterConnections; i++) {
    connection = &config->testerConnections[i];
    if (DOIP_CON_CLOSED != connection->context->state) {
      if (connection->context->InactivityTimer > 0) {
        connection->context->InactivityTimer--;
        if (0 == connection->context->InactivityTimer) {
          printf("Tester SoCon %d InactivityTimer timeout\n", i);
          SoAd_CloseSoCon(connection->SoConId, TRUE);
          doipForgetDiagMsg(connection);
          memset(connection->context, 0, sizeof(DoIP_TesterConnectionContextType));
        }
      }
    }
  }
}

static void doipHandleAliveCheckResponseTimer(void) {
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection;
  int i;

  for (i = 0; i < config->MaxTesterConnections; i++) {
    connection = &config->testerConnections[i];
    if (DOIP_CON_CLOSED != connection->context->state) {
      if (connection->context->AliveCheckResponseTimer > 0) {
        connection->context->AliveCheckResponseTimer--;
        if (0 == connection->context->AliveCheckResponseTimer) {
          printf("Tester SoCon %d AliveCheckResponseTimer timeout\n", i);
          SoAd_CloseSoCon(connection->SoConId, TRUE);
          memset(connection->context, 0, sizeof(DoIP_TesterConnectionContextType));
        }
      }
    }
  }
}

static void doipHandleVehicleAnnouncement(void) {
  const DoIP_ConfigType *config = DOIP_CONFIG;
  int i;
  PduInfoType PduInfo;
  uint8_t logcamMsg[DOIP_SHORT_MSG_LOCAL_BUFFER_SIZE];

  for (i = 0; i < config->numOfUdpVehicleAnnouncementConnections; i++) {
    if (DOIP_CON_CLOSED != config->UdpVehicleAnnouncementConnections[i].context->state) {
      if (config->UdpVehicleAnnouncementConnections[i].context->AnnouncementTimer > 0) {
        config->UdpVehicleAnnouncementConnections[i].context->AnnouncementTimer--;
        if (0 == config->UdpVehicleAnnouncementConnections[i].context->AnnouncementTimer) {
          printf("Udp %d VehicleAnnouncement\n", i);
          PduInfo.SduLength = doipSetupVehicleAnnouncementResponse(logcamMsg);
          PduInfo.SduDataPtr = logcamMsg;
          PduInfo.MetaDataPtr = NULL;
          printf("Udp length:%d\n", PduInfo.SduLength);
          (void)SoAd_IfTransmit(config->UdpVehicleAnnouncementConnections[i].SoAdTxPdu, &PduInfo);
          config->UdpVehicleAnnouncementConnections[i].context->AnnouncementCounter++;
          if (config->UdpVehicleAnnouncementConnections[i].context->AnnouncementCounter <
              config->VehicleAnnouncementCount) {
            config->UdpVehicleAnnouncementConnections[i].context->AnnouncementTimer =
              config->VehicleAnnouncementInterval;
          }
        }
      }
    }
  }
  printf("finish doipHandleVehicleAnnouncement\n");
}

Std_ReturnType doipTpSendResponse(PduIdType RxPduId, uint8_t *data, PduLengthType length) {
  PduInfoType PduInfo;
  Std_ReturnType ret;
  if (length > 0) {
    PduInfo.SduDataPtr = data;
    PduInfo.MetaDataPtr = NULL;
    PduInfo.SduLength = length;
    ret = SoAd_TpTransmit(RxPduId, &PduInfo);
  } else {
    ret = E_NOT_OK;
  }
  return ret;
}

static void doipHandleDiagMsgResponse(void) {
  Std_ReturnType ret = E_NOT_OK;
  BufReq_ReturnType bret;
  PduInfoType PduInfo;
  PduLengthType left;
  PduIdType TxPduId;
  const DoIP_TargetAddressType *TargetAddressRef;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection = NULL;
  int i;
  uint8_t *res;
  uint32_t resLen;

  for (i = 0; i < (NULL == connection) && (config->MaxTesterConnections); i++) {
    connection = &config->testerConnections[i];
    if (DOIP_CON_CLOSED != connection->context->state) {
      TargetAddressRef = connection->context->msg.TargetAddressRef;
      if (NULL != TargetAddressRef) {
        if (DOIP_MSG_TX == connection->context->msg.state) {
          TxPduId = TargetAddressRef->TxPduId;
          connection = &config->testerConnections[i];
          resLen = connection->context->msg.TpSduLength - connection->context->msg.index;
          res = malloc(resLen);
          if (NULL != res) {
            ret = E_OK;
          }
        }
      }
    }
  }

  /* NEED PDUR!*/
  if (E_OK == ret) {
    PduInfo.SduDataPtr = res;
    PduInfo.SduLength = resLen;
    if (PduInfo.SduLength >
        (connection->context->msg.TpSduLength - connection->context->msg.index)) {
      PduInfo.SduLength = connection->context->msg.TpSduLength - connection->context->msg.index;
    }
    //bret = PduR_DoIPCopyTxData(TxPduId, &PduInfo, NULL, &left);
    bret = BUFREQ_OK;
    if (BUFREQ_OK == bret) {
      ret = doipTpSendResponse(connection->SoAdTxPdu, PduInfo.SduDataPtr, PduInfo.SduLength);
      if (E_OK != ret) {
        connection->context->msg.state = DOIP_MSG_IDLE;
        //PduR_DoIPTxConfirmation(TxPduId, E_NOT_OK);
      }
    } else {
      ret = E_NOT_OK;
    }

    if (E_OK == ret) {
      if (PduInfo.SduLength >= connection->context->msg.TpSduLength) {
        connection->context->msg.state = DOIP_MSG_IDLE;
        //PduR_DoIPTxConfirmation(TxPduId, E_OK);
        printf("[%d] send UDS response done\n", TxPduId);
      } else {
        printf ("[%d] send UDS response on going\n", TxPduId);
      }
    }

    free(res);
  }
  printf("finish doipHandleDiagMsgResponse\n");
}

static Std_ReturnType doipCheckTesterConnectionAlive(const DoIP_TesterConnectionType *connection) {
  Std_ReturnType ret = E_OK;
  uint8_t data[DOIP_HEADER_LENGTH];
  const DoIP_ConfigType *config = DOIP_CONFIG;

  if (0 == connection->context->AliveCheckResponseTimer) {
    /* no request is already on going, start one */
    doipFillHeader(data, DOIP_ALIVE_CHECK_REQUEST, 0);
    connection->context->isAlive = FALSE;
    connection->context->AliveCheckResponseTimer = config->AliveCheckResponseTimeout;
    ret = doipTpSendResponse(connection->SoAdTxPdu, data, DOIP_HEADER_LENGTH);
  }
  return ret;
}

static Std_ReturnType doipSocketHandler(const DoIP_TesterConnectionType *tstcon, uint8_t *resCode) {
  Std_ReturnType ret = E_OK;
  boolean isAliveCheckOngoing = FALSE;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  int i;
  const DoIP_TesterConnectionType *connection;
  for (i = 0; (i < config->MaxTesterConnections) && (E_OK == ret); i++) {
    connection = &config->testerConnections[i];
    if ((tstcon != connection) && (DOIP_CON_OPEN == connection->context->state) &&
        (tstcon->context->ramgr.tester == connection->context->TesterRef)) {
      printf("Tester 0x%x is already active on connection %d, check alive\n",
                   tstcon->context->ramgr.tester->TesterSA, i);
      ret = doipCheckTesterConnectionAlive(connection);
      if (E_OK == ret) {
        isAliveCheckOngoing = TRUE;
      } else {
        /* this connection is closed by Tester, close it */
        printf("Tester SoCon %d not alive, close it\n", i);
        SoAd_CloseSoCon(connection->SoConId, TRUE);
        memset(connection->context, 0, sizeof(DoIP_TesterConnectionContextType));
        ret = E_OK;
      }
    }
  }

  if (E_OK == ret) {
    if (isAliveCheckOngoing) {
      ret = DOIP_E_PENDING;
    }
  }

  return ret;
}

static Std_ReturnType doipSocketHandler2(const DoIP_TesterConnectionType *tstcon,
                                         uint8_t *resCode) {
  Std_ReturnType ret = E_OK;
  boolean isAliveCheckOngoing = FALSE;
  int i;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection;
  for (i = 0; (i < config->MaxTesterConnections) && (E_OK == ret); i++) {
    connection = &config->testerConnections[i];
    if ((tstcon != connection) && (DOIP_CON_OPEN == connection->context->state) &&
        (tstcon->context->ramgr.tester == connection->context->TesterRef)) {
      printf("Tester 0x%x is already active on connection %d, check alive response\n",
                   tstcon->context->ramgr.tester->TesterSA, i);
      if (TRUE == connection->context->isAlive) {
        /* @SWS_DoIP_00106 */
        *resCode = 0x02;
        ret = DOIP_E_NOT_OK;
        break;
      } else if (connection->context->AliveCheckResponseTimer > 0) {
        isAliveCheckOngoing = TRUE;
      } else {
      }
    }
  }

  if (ret == E_OK) {
    if (isAliveCheckOngoing) {
      ret = DOIP_E_PENDING;
    }
  }

  return ret;
}


static Std_ReturnType doipRoutingActivationManager(const DoIP_TesterConnectionType *connection,
                                                   DoIP_MsgType *msg, uint8_t *resCode) {
  Std_ReturnType ret = E_OK;
  const DoIP_TesterType *tester = connection->context->ramgr.tester;
  uint8_t raid = connection->context->ramgr.raid;
  const DoIP_RoutingActivationType *ra = tester->RoutingActivationRefs[raid];
  boolean Authentified = FALSE;
  boolean Confirmed = FALSE;
  boolean ongoing = FALSE;

  do {
    switch (connection->context->ramgr.state) {
    case DOIP_RA_SOCKET_HANDLER:
      ret = doipSocketHandler(connection, resCode);
      if (DOIP_E_NOT_OK != ret) {
        connection->context->ramgr.state = DOIP_RA_SOCKET_HANDLER_2;
        ongoing = TRUE;
      } else {
        ongoing = FALSE;
      }
      break;
    case DOIP_RA_SOCKET_HANDLER_2:
      ret = doipSocketHandler2(connection, resCode);
      if (E_OK == ret) {
        connection->context->ramgr.state = DOIP_RA_CHECK_AUTHENTICATION;
        ongoing = TRUE;
      } else {
        ongoing = FALSE;
      }
      break;
    case DOIP_RA_CHECK_AUTHENTICATION:
      ongoing = FALSE;
      ret = ra->AuthenticationCallback(&Authentified, connection->context->ramgr.OEM,
                                       &msg->res[DOIP_HEADER_LENGTH + 9]);
      if ((E_OK == ret) && (Authentified)) {
        /* pass, going to check confirmed or not */
        connection->context->ramgr.state = DOIP_RA_CHECK_CONFIRMATION;
        ongoing = TRUE;
      } else if (DOIP_E_PENDING == ret) {
        /* @SWS_DoIP_00110 */
      } else {
        *resCode = 0x04; /* @SWS_DoIP_00111 */
        ret = DOIP_E_NOT_OK;
      }
      break;
    case DOIP_RA_CHECK_CONFIRMATION:
      ongoing = FALSE;
      ret = ra->ConfirmationCallback(&Confirmed, connection->context->ramgr.OEM,
                                     &msg->res[DOIP_HEADER_LENGTH + 9]);
      if ((E_OK == ret) && (Confirmed)) {
        *resCode = 0x10; /* @SWS_DoIP_00113 */
        connection->context->RAMask |= (1u << raid);
        connection->context->TesterRef = tester;
        connection->context->ramgr.state = DOIP_RA_IDLE;
      } else if (DOIP_E_PENDING == ret) {
        *resCode = 0x11; /* @SWS_DoIP_00114 */
      } else {
        *resCode = 0x05; /* @SWS_DoIP_00274 */
        ret = DOIP_E_NOT_OK;
      }
      break;
    default:
      ongoing = FALSE;
      *resCode = 101; /* internall state error */
      ret = DOIP_E_NOT_OK;
      break;
    }
  } while (ongoing);

  return ret;
}

static void doipBuildRountineActivationResponse(const DoIP_TesterConnectionType *connection,
                                                DoIP_MsgType *msg, Std_ReturnType ret, uint16_t sa,
                                                uint8_t resCode) {
  const DoIP_ConfigType *config = DOIP_CONFIG;
  printf("doipBuildRountineActivationResponse\n");
  doipFillHeader(msg->res, DOIP_ROUTING_ACTIVATION_RESPONSE, 13);
  msg->res[DOIP_HEADER_LENGTH + 0] = (sa >> 8) & 0xFF; /* Logical Address Tester */
  msg->res[DOIP_HEADER_LENGTH + 1] = sa & 0xFF;

  msg->res[DOIP_HEADER_LENGTH + 2] =
    (config->LogicalAddress >> 8) & 0xFF; /* Logical address of DoIP entity */
  msg->res[DOIP_HEADER_LENGTH + 3] = config->LogicalAddress & 0xFF;
  msg->res[DOIP_HEADER_LENGTH + 4] = resCode;
  msg->resLen = DOIP_HEADER_LENGTH + 13;

  if ((DOIP_E_PENDING == ret) && (0x11 == resCode)) {
    (void)doipTpSendResponse(connection->SoAdTxPdu, msg->res, msg->resLen); /* @SWS_DoIP_00114 */
  }
}

static const DoIP_TesterType *doipFindTester(uint16_t sourceAddress) {
  const DoIP_TesterType *tester = NULL;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  int i;

  for (i = 0; i < config->numOfTesters; i++) {
    if (config->testers[i].TesterSA == sourceAddress) {
      tester = &config->testers[i];
      break;
    }
  }

  return tester;
}

static const DoIP_RoutingActivationType *
doipFindRoutingActivation(const DoIP_TesterType *tester, uint8_t activationType, uint8_t *raid) {
  const DoIP_RoutingActivationType *ra = NULL;
  int i;

  for (i = 0; i < tester->numOfRoutingActivations; i++) {
    if (tester->RoutingActivationRefs[i]->Number == activationType) {
      ra = tester->RoutingActivationRefs[i];
      if (NULL != raid) {
        *raid = i;
      }
    }
  }

  return ra;
};

static Std_ReturnType doipHandleRoutineActivation(PduIdType RxPduId, DoIP_MsgType *msg,
                                                  uint8_t *nack) {
  Std_ReturnType ret = E_OK;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection =
    &config->testerConnections[config->RxPduIdToConnectionMap[RxPduId]];
  const DoIP_TesterType *tester = NULL;
  const DoIP_RoutingActivationType *ra = NULL;
  uint8_t raid;
  uint16_t sourceAddress;
  uint8_t activationType;
  uint8_t resCode = 0xFF;
  if ((7 == msg->payloadLength) || (11 == msg->payloadLength)) { /* @SWS_DoIP_00117 */
    sourceAddress = ((uint16_t)msg->req[0] << 8) + msg->req[1];
    activationType = msg->req[2];
    tester = doipFindTester(sourceAddress);
    if (NULL == tester) {
      resCode = 0x00; /* @SWS_DoIP_00104 */
    } else {
      ra = doipFindRoutingActivation(tester, activationType, &raid);
      if (NULL == ra) {
        resCode = 0x06; /* SWS_DoIP_00160 */
      }
    }
    if (NULL != ra) {
      if ((0 != connection->context->RAMask) && (connection->context->TesterRef != tester)) {
        resCode = 0x02; /* @SWS_DoIP_00106*/
        ret = DOIP_E_NOT_OK;
      }
    } else {
      ret = DOIP_E_NOT_OK;
    }

    if (E_OK == ret) {
      connection->context->ramgr.tester = tester;
      connection->context->ramgr.raid = raid;
      memcpy(connection->context->ramgr.OEM, &msg->req[7], 4);
      connection->context->ramgr.state = DOIP_RA_SOCKET_HANDLER;
      ret = doipRoutingActivationManager(connection, msg, &resCode);
    }

    doipBuildRountineActivationResponse(connection, msg, ret, sourceAddress, resCode);
    if (DOIP_E_NOT_OK == ret) {
      connection->context->InactivityTimer = 1; /* close it the next MainFunction */
    }
  } else {
    *nack = DOIP_INVALID_PAYLOAD_LENGTH_NACK;
    ret = E_NOT_OK;
  }
  printf("[%d] handle Routine Activation Request\n", RxPduId);
  return ret;
}


static void doipHandleRoutineActivationMain() {
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection;
  int i;
  uint8_t resCode = 0xFE;
  Std_ReturnType ret;
  DoIP_MsgType msg;
  uint8_t logcalMsg[DOIP_SHORT_MSG_LOCAL_BUFFER_SIZE];
  msg.res = logcalMsg;
  msg.resLen = sizeof(logcalMsg);

  for (i = 0; i < config->MaxTesterConnections; i++) {
    connection = &config->testerConnections[i];
    if (DOIP_CON_CLOSED != connection->context->state) {
      if (DOIP_RA_IDLE != connection->context->ramgr.state) {
        ret = doipRoutingActivationManager(connection, &msg, &resCode);
        doipBuildRountineActivationResponse(connection, &msg, ret,
                                            connection->context->ramgr.tester->TesterSA, resCode);
        if (DOIP_E_NOT_OK == ret) {
          connection->context->InactivityTimer = 1; /* close it the next MainFunction */
        }

        if (E_OK == ret) {
          connection->context->InactivityTimer = config->GeneralInactivityTime;
          (void)doipTpSendResponse(connection->SoAdTxPdu, msg.res, msg.resLen);
        } else if (DOIP_E_PENDING == ret) {
        } else {
          (void)doipTpSendResponse(connection->SoAdTxPdu, msg.res, msg.resLen);
          ret = BUFREQ_E_NOT_OK;
        }
      }
    }
  }
  printf("finish doipHandleRoutineActivationMain\n");
}

void DoIP_ActivationLineSwitchActive(void) {
  const DoIP_ConfigType *config = DOIP_CONFIG;
  DoIP_ContextType *context = &DoIP_Context;
  int i;

  if (DOIP_ACTIVATION_LINE_INACTIVE == context->ActivationLineState) {
    printf("switch to active\n");
    context->ActivationLineState = DOIP_ACTIVATION_LINE_ACTIVE;
    /* @SWS_DoIP_00204 */
    for (i = 0; i < config->numOfTcpConnections; i++) {
      if (config->TcpConnections[i].RequestAddressAssignment) {
        SoAd_OpenSoCon(config->TcpConnections[i].SoConId);
      }
    }
    for (i = 0; i < config->numOfUdpConnections; i++) {
      if (config->UdpConnections[i].RequestAddressAssignment) {
        SoAd_OpenSoCon(config->UdpConnections[i].SoConId);
      }
    }
    for (i = 0; i < config->numOfUdpVehicleAnnouncementConnections; i++) {
      if (config->UdpVehicleAnnouncementConnections[i].RequestAddressAssignment) {
        SoAd_OpenSoCon(config->UdpVehicleAnnouncementConnections[i].SoConId);
      }
    }
  }
}

void DoIP_SoConModeChg(SoAd_SoConIdType SoConId, SoAd_SoConModeType Mode) {
  Std_ReturnType ret = E_NOT_OK;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  DoIP_ContextType *context = &DoIP_Context;
  int i;

  for (i = 0; i < config->numOfTcpConnections; i++) {
    if (SoConId == config->TcpConnections[i].SoConId) {
      printf("Tcp %d SoCon Mode %d\n", i, Mode);
      ret = E_OK;
      break;
    }
  }

  for (i = 0; (i < config->numOfUdpConnections) && (E_NOT_OK == ret); i++) {
    if (SoConId == config->UdpConnections[i].SoConId) {
      printf("Udp SoCon %d Mode %d\n", i, Mode);
      ret = E_OK;
    }
  }

  for (i = 0; i < config->numOfUdpVehicleAnnouncementConnections; i++) {
    if (SoConId == config->UdpVehicleAnnouncementConnections[i].SoConId) {
      if (SOAD_SOCON_ONLINE == Mode) {
        assert(DOIP_ACTIVATION_LINE_ACTIVE == context->ActivationLineState);
        config->UdpVehicleAnnouncementConnections[i].context->state = DOIP_CON_OPEN;
        config->UdpVehicleAnnouncementConnections[i].context->AnnouncementTimer =
          config->InitialVehicleAnnouncementTime;
        config->UdpVehicleAnnouncementConnections[i].context->AnnouncementCounter = 0;
      }
      printf("UdpVehicleAnnouncement SoCon %d Mode %d\n", i, Mode);
      ret = E_OK;
      break;
    }
  }

  for (i = 0; (i < config->MaxTesterConnections) && (E_NOT_OK == ret); i++) {
    if (SoConId == config->testerConnections[i].SoConId) {
      if (SOAD_SOCON_ONLINE == Mode) {
        assert(DOIP_ACTIVATION_LINE_ACTIVE == context->ActivationLineState);
        memset(config->testerConnections[i].context, 0, sizeof(DoIP_TesterConnectionContextType));
        config->testerConnections[i].context->InactivityTimer = config->InitialInactivityTime;
        config->testerConnections[i].context->state = DOIP_CON_OPEN;
      }
      printf("Tester SoCon %d Mode %d\n", i, Mode);
      ret = E_OK;
    }
  }
}

BufReq_ReturnType DoIP_SoAdTpStartOfReception(PduIdType RxPduId, const PduInfoType *info,
                                              PduLengthType TpSduLength,
                                              PduLengthType *bufferSizePtr) {
  BufReq_ReturnType ret = BUFREQ_OK;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  DoIP_ContextType *context = &DoIP_Context;

  printf("DoIP_SoAdTpStartOfReception called\n");
  if (RxPduId < DOIP_CONFIG->numOfRxPduIds) {
    if (DOIP_ACTIVATION_LINE_INACTIVE == context->ActivationLineState) {
      /* @SWS_DoIP_00202 */
      ret = BUFREQ_E_NOT_OK;
    } else {
      if (DOIP_CONFIG->RxPduIdToConnectionMap[RxPduId] >= config->MaxTesterConnections) {
        /* @SWS_DoIP_00101 */
        ret = BUFREQ_E_NOT_OK;
        printf("RxPduId %d get invalid connection map\n", RxPduId);
      }
    }
  } else {
    printf("TpStartOfReception with invalid RxPduId %d\n", RxPduId);
    ret = BUFREQ_E_NOT_OK;
  }

  return ret;
}

static Std_ReturnType doipDecodeMsg(const PduInfoType *PduInfoPtr, DoIP_MsgType *msg) {
  Std_ReturnType ret = E_NOT_OK;

  if ((PduInfoPtr->SduLength >= DOIP_HEADER_LENGTH) && (PduInfoPtr->SduDataPtr != NULL)) {
    /* @SWS_DoIP_00005, @SWS_DoIP_00006 */
    if ((DOIP_PROTOCOL_VERSION == PduInfoPtr->SduDataPtr[0]) &&
        (PduInfoPtr->SduDataPtr[0] = ((~PduInfoPtr->SduDataPtr[1])) & 0xFF)) {

      msg->payloadType = ((uint16_t)PduInfoPtr->SduDataPtr[2] << 8) + PduInfoPtr->SduDataPtr[3];
      msg->payloadLength = ((uint32_t)PduInfoPtr->SduDataPtr[4] << 24) +
                           ((uint32_t)PduInfoPtr->SduDataPtr[5] << 16) +
                           ((uint32_t)PduInfoPtr->SduDataPtr[6] << 8) + PduInfoPtr->SduDataPtr[7];
      msg->req = &PduInfoPtr->SduDataPtr[DOIP_HEADER_LENGTH];
      msg->reqLen = PduInfoPtr->SduLength - DOIP_HEADER_LENGTH;
      ret = E_OK;
    }
  }
  return ret;
}

static Std_ReturnType doipHandleAliveCheckRequest(PduIdType RxPduId, DoIP_MsgType *msg,
                                                  uint8_t *nack) {
  Std_ReturnType ret = E_OK;
  const DoIP_ConfigType *config = DOIP_CONFIG;

  if (0 == msg->payloadLength) {
    doipFillHeader(msg->res, DOIP_ALIVE_CHECK_RESPONSE, 2);
    msg->res[DOIP_HEADER_LENGTH + 0] = (config->LogicalAddress >> 8) & 0xFF;
    msg->res[DOIP_HEADER_LENGTH + 1] = config->LogicalAddress & 0xFF;
    msg->resLen = DOIP_HEADER_LENGTH + 2;
  } else {
    *nack = DOIP_INVALID_PAYLOAD_LENGTH_NACK;
    ret = E_NOT_OK;
  }

  printf("[%d] handle Alive Check Request\n", RxPduId);

  return ret;
}

static Std_ReturnType doipHandleAliveCheckResponse(PduIdType RxPduId, DoIP_MsgType *msg,
                                                   uint8_t *nack) {
  Std_ReturnType ret = E_OK;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection =
    &config->testerConnections[config->RxPduIdToConnectionMap[RxPduId]];
  uint16_t sa;

  if (2 == msg->payloadLength) {
    sa = ((uint16_t)msg->req[0] << 8) + msg->req[1];
    if (sa != connection->context->TesterRef->TesterSA) {
      /* @SWS_DoIP_00141 */
      printf("sa %X not right, expected %X, close it\n", sa,
                    connection->context->TesterRef->TesterSA);
      ret = DOIP_E_NOT_OK_SILENT;
      connection->context->InactivityTimer = 1; /* close it the next MainFunction */
    } else {
      connection->context->isAlive = TRUE;
      connection->context->AliveCheckResponseTimer = 0;
      msg->resLen = 0; /* no response */
    }
  } else {
    *nack = DOIP_INVALID_PAYLOAD_LENGTH_NACK;
    ret = E_NOT_OK;
  }

  printf("[%d] handle Alive Check Response\n", RxPduId);

  return ret;
}

static void doipRememberDiagMsg(const DoIP_TesterConnectionType *connection, DoIP_MsgType *msg) {
  PduLengthType bufferSize;
  if (DOIP_MSG_IDLE == connection->context->msg.state) {
    bufferSize = msg->payloadLength - 4;
    if (bufferSize > connection->context->TesterRef->NumByteDiagAckNack) {
      bufferSize = connection->context->TesterRef->NumByteDiagAckNack;
    }
    bufferSize += DOIP_HEADER_LENGTH + 5; /* for header */
    if (NULL != connection->context->msg.req) {
      free(connection->context->msg.req);
    }
    connection->context->msg.req = malloc(bufferSize);
    if (NULL != connection->context->msg.req) {
      /* save sa&ta and uds message */
      memcpy(&(connection->context->msg.req[DOIP_HEADER_LENGTH]), msg->req, 4);
      memcpy(&(connection->context->msg.req[DOIP_HEADER_LENGTH + 5]), &msg->req[4],
             msg->reqLen - 4);
    }
  } else {
    if (NULL != connection->context->msg.req) {
      if (connection->context->msg.index < connection->context->TesterRef->NumByteDiagAckNack) {
        bufferSize =
          connection->context->TesterRef->NumByteDiagAckNack - connection->context->msg.index;
        if (bufferSize > msg->reqLen) {
          bufferSize = msg->reqLen;
        }
        memcpy(
          &(connection->context->msg.req[DOIP_HEADER_LENGTH + 5 + connection->context->msg.index]),
          msg->req, bufferSize);
      }
    }
  }
}

static const DoIP_TargetAddressType *
doipFindTargetAddress(const DoIP_TesterType *tester, const DoIP_TesterConnectionType *connection,
                      uint16_t ta) {
  const DoIP_TargetAddressType *TargetAddressRef = NULL;
  const DoIP_RoutingActivationType *ra;
  int i, j;

  for (i = 0; (i < tester->numOfRoutingActivations) && (NULL == TargetAddressRef); i++) {
    if (connection->context->RAMask & (1u << i)) {
      ra = tester->RoutingActivationRefs[i];
      for (j = 0; (j < ra->numOfTargetAddressRefs) && (NULL == TargetAddressRef); j++) {
        if (ta == ra->TargetAddressRefs[j]->TargetAddress) {
          TargetAddressRef = ra->TargetAddressRefs[j];
        }
      }
    }
  }

  return TargetAddressRef;
}

static void doipReplyDiagMsg(const DoIP_TesterConnectionType *connection, DoIP_MsgType *msg,
                             uint8_t resCode) {
  PduLengthType resLen = 5;
  uint16_t payloadType = DOIP_DIAGNOSTIC_MESSAGE_POSITIVE_ACK;

  printf("doipReplyDiagMsg\n");
  if (NULL != connection->context->msg.req) {
    /* @SWS_DoIP_00138 */
    msg->res = connection->context->msg.req;
    resLen = connection->context->msg.index;
    if (resLen > connection->context->TesterRef->NumByteDiagAckNack) {
      resLen = connection->context->TesterRef->NumByteDiagAckNack;
    }
    resLen += 5;
  }

  if (0x0 != resCode) {
    payloadType = DOIP_DIAGNOSTIC_MESSAGE_NEGATIVE_ACK;
  }
  doipFillHeader(msg->res, payloadType, resLen);
  msg->res[DOIP_HEADER_LENGTH + 4] = resCode;
  msg->resLen = DOIP_HEADER_LENGTH + resLen;
}

static Std_ReturnType doipHandleDiagnosticMessage(PduIdType RxPduId, DoIP_MsgType *msg,
                                                  uint8_t *nack) {
  Std_ReturnType ret = E_OK;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  const DoIP_TesterConnectionType *connection =
    &config->testerConnections[config->RxPduIdToConnectionMap[RxPduId]];
  const DoIP_TargetAddressType *TargetAddressRef;
  uint16_t sa, ta;
  uint8_t resCode = 0xFF;
  BufReq_ReturnType bufReq;
  PduInfoType pduInfo;
  PduLengthType bufferSize;

  if (DOIP_MSG_IDLE == connection->context->msg.state) {
    if (msg->payloadLength < 5) {
      *nack = DOIP_INVALID_PAYLOAD_LENGTH_NACK;
      ret = E_NOT_OK;
    }

    if (msg->reqLen < 5) {
      *nack = 0x99; /* TODO: internal error */
      ret = E_NOT_OK;
    }
  }

  if (E_OK == ret) {
    doipRememberDiagMsg(connection, msg);
    if ((0u == connection->context->RAMask) || (NULL == connection->context->TesterRef)) {
      resCode = 0x02; /* @SWS_DoIP_00123 */
      printf("not activated\n");
      ret = DOIP_E_NOT_OK;
    }
  }

  if (E_OK == ret) {
    if (DOIP_MSG_IDLE == connection->context->msg.state) {
      sa = ((uint16_t)msg->req[0] << 8) + msg->req[1];
      ta = ((uint16_t)msg->req[2] << 8) + msg->req[3];

      if (sa != connection->context->TesterRef->TesterSA) {
        printf("sa %X not right, expected %X\n", sa, connection->context->TesterRef->TesterSA);
        resCode = 0x02; /* @SWS_DoIP_00123 */
        ret = DOIP_E_NOT_OK;
      } else {
        TargetAddressRef = doipFindTargetAddress(connection->context->TesterRef, connection, ta);
        if (NULL == TargetAddressRef) {
          resCode = 0x06; /* @SWS_DoIP_00127 */
          ret = DOIP_E_NOT_OK;
        }
      }
    }
  }

  if (E_OK == ret) {
    if (DOIP_MSG_IDLE == connection->context->msg.state) {
      connection->context->msg.TargetAddressRef = TargetAddressRef;
      pduInfo.SduDataPtr = (uint8_t *)&msg->req[4];
      pduInfo.SduLength = msg->reqLen - 4;
      //bufReq = PduR_DoIPStartOfReception(TargetAddressRef->RxPduId, &pduInfo,
      //                                   msg->payloadLength - 4, &bufferSize);
      bufReq = BUFREQ_OK;
      if (bufReq != BUFREQ_OK) {
        resCode = 0x08; /* @SWS_DoIP_00174 */
        ret = DOIP_E_NOT_OK;
      } else {
        //forget PduR
        //bufReq = PduR_DoIPCopyRxData(TargetAddressRef->RxPduId, &pduInfo, &bufferSize);
        bufReq = BUFREQ_OK;
        if (bufReq != BUFREQ_OK) {
          connection->context->msg.state = DOIP_MSG_IDLE;
          //PduR_DoIPRxIndication(TargetAddressRef->RxPduId, E_NOT_OK);
          resCode = 0x08; /* @SWS_DoIP_00174 */
          ret = DOIP_E_NOT_OK;
        } else {
          connection->context->msg.state = DOIP_MSG_RX;
          connection->context->msg.index = msg->reqLen - 4;
          connection->context->msg.TpSduLength = msg->payloadLength - 4;
        }
      }
    } else {
      TargetAddressRef = connection->context->msg.TargetAddressRef;
      assert(TargetAddressRef != NULL);
      pduInfo.SduDataPtr = (uint8_t *)msg->req;
      pduInfo.SduLength = msg->reqLen;
      //bufReq = PduR_DoIPCopyRxData(TargetAddressRef->RxPduId, &pduInfo, &bufferSize);

      if (bufReq != BUFREQ_OK) {
        //PduR_DoIPRxIndication(TargetAddressRef->RxPduId, E_NOT_OK);
        connection->context->msg.state = DOIP_MSG_IDLE;
        resCode = 0x08; /* @SWS_DoIP_00174 */
        ret = DOIP_E_NOT_OK;
      } else {
        connection->context->msg.index += msg->reqLen;
      }
    }
  }

  if (E_OK == ret) {
    if (connection->context->msg.index >= connection->context->msg.TpSduLength) {
      connection->context->msg.state = DOIP_MSG_IDLE;
      //PduR_DoIPRxIndication(TargetAddressRef->RxPduId, E_OK);
      doipFillHeader(msg->res, DOIP_DIAGNOSTIC_MESSAGE_POSITIVE_ACK, 5);
      msg->res[DOIP_HEADER_LENGTH + 4] = 0x0; /* ack code */
      msg->resLen = DOIP_HEADER_LENGTH + 5;
      doipReplyDiagMsg(connection, msg, 0x0);
    } else {
      connection->context->msg.state = DOIP_MSG_RX;
      ret = DOIP_E_PENDING;
    }
  }

  if (ret == DOIP_E_NOT_OK) {
    doipReplyDiagMsg(connection, msg, resCode);
  }

  printf("[%d] handle Diagnostic Message\n", RxPduId);

  return ret;
}

static Std_ReturnType doipTpStartOfReception(PduIdType RxPduId, DoIP_MsgType *msg, uint8_t *nack) {
  Std_ReturnType ret = E_OK;

  switch (msg->payloadType) {
  case DOIP_ROUTING_ACTIVATION_REQUEST:
    if (msg->reqLen >= msg->payloadLength) {
      ret = doipHandleRoutineActivation(RxPduId, msg, nack);
    } else {
      *nack = DOIP_TOO_MUCH_PAYLOAD_NACK;
      ret = E_NOT_OK;
    }
    break;
  case DOIP_ALIVE_CHECK_REQUEST:
    if (msg->reqLen >= msg->payloadLength) {
      ret = doipHandleAliveCheckRequest(RxPduId, msg, nack);
    } else {
      *nack = DOIP_TOO_MUCH_PAYLOAD_NACK;
      ret = E_NOT_OK;
    }
    break;
  case DOIP_ALIVE_CHECK_RESPONSE:
    if (msg->reqLen >= msg->payloadLength) {
      ret = doipHandleAliveCheckResponse(RxPduId, msg, nack);
    } else {
      *nack = DOIP_TOO_MUCH_PAYLOAD_NACK;
      ret = E_NOT_OK;
    }
    break;
  case DOIP_DIAGNOSTIC_MESSAGE:
    ret = doipHandleDiagnosticMessage(RxPduId, msg, nack);
    break;
  default:
    *nack = DOIP_UNKNOW_PAYLOAD_TYPE_NACK;
    ret = E_NOT_OK;
    break;
  }

  return ret;
}

static Std_ReturnType doipTpCopyRxData(PduIdType RxPduId, DoIP_MsgType *msg, uint8_t *nack) {
  Std_ReturnType ret = E_OK;
  /* only DIAGNOSTIC_MESSAGE allow this */

  ret = doipHandleDiagnosticMessage(RxPduId, msg, nack);

  return ret;
}

BufReq_ReturnType DoIP_SoAdTpCopyRxData(PduIdType RxPduId, const PduInfoType *PduInfoPtr,
                                        PduLengthType *bufferSizePtr) {
  BufReq_ReturnType ret = BUFREQ_OK;
  Std_ReturnType r = E_OK;
  const DoIP_ConfigType *config = DOIP_CONFIG;
  DoIP_ContextType *context = &DoIP_Context;
  const DoIP_TesterConnectionType *connection;
  uint8_t nack = DOIP_INVALID_PROTOCOL_NACK;
  DoIP_MsgType msg;
  uint8_t localMsg[DOIP_SHORT_MSG_LOCAL_BUFFER_SIZE];
  msg.res = localMsg;
  msg.resLen = sizeof(localMsg);

  if (RxPduId < DOIP_CONFIG->numOfRxPduIds) {
    if (DOIP_ACTIVATION_LINE_INACTIVE == context->ActivationLineState) {
      ret = BUFREQ_E_NOT_OK;
      r = DOIP_E_NOT_OK_SILENT;
    } else {
      if (DOIP_CONFIG->RxPduIdToConnectionMap[RxPduId] < config->MaxTesterConnections) {
        connection = &config->testerConnections[DOIP_CONFIG->RxPduIdToConnectionMap[RxPduId]];
      } else {
        ret = BUFREQ_E_NOT_OK;
        r = DOIP_E_NOT_OK_SILENT;
      }
    }
  } else {
    printf("TpStartOfReception with invalid RxPduId %d\n", RxPduId);
    ret = BUFREQ_E_NOT_OK;
    r = DOIP_E_NOT_OK_SILENT;
  }

  if (E_OK == r) {
    if (DOIP_MSG_IDLE == connection->context->msg.state) {
      r = doipDecodeMsg(PduInfoPtr, &msg);
      if (E_OK == r) {
        r = doipTpStartOfReception(RxPduId, &msg, &nack);
      }
    } else {
      msg.req = PduInfoPtr->SduDataPtr;
      msg.reqLen = PduInfoPtr->SduLength;
      r = doipTpCopyRxData(RxPduId, &msg, &nack);
    }
  }

  if (E_NOT_OK == r) {
    doipFillHeader(msg.res, DOIP_GENERAL_HEADER_NEGATIVE_ACK, 1);
    msg.res[DOIP_HEADER_LENGTH] = nack;
    msg.resLen = DOIP_HEADER_LENGTH + 1;
  }

  if (E_OK == r) {
    connection->context->InactivityTimer = config->GeneralInactivityTime;
    (void)doipTpSendResponse(connection->SoAdTxPdu, msg.res, msg.resLen);
  } else if (DOIP_E_NOT_OK_SILENT == r) {
    /* slient */
  } else if (DOIP_E_PENDING == r) {
    /* TODO */
  } else {
    (void)doipTpSendResponse(connection->SoAdTxPdu, msg.res, msg.resLen);
    ret = BUFREQ_E_NOT_OK;
  }

  return ret;
}



static Std_ReturnType doIpIfHandleMessage(PduIdType RxPduId, DoIP_MsgType *msg, uint8_t *nack) {
  Std_ReturnType ret = E_OK;
  printf("doIpIfHandleMessage! msg->payloadType:0x%x", msg->payloadType);
  /*switch (msg->payloadType) {
  case DOIP_VID_REQUEST:
    ret = doipHandleVIDRequest(RxPduId, msg, nack);
    break;
  case DOIP_VID_REQUEST_WITH_EID:
    ret = doipHandleVIDRequestWithEID(RxPduId, msg, nack);
    break;
  case DOIP_VID_REQUEST_WITH_VIN:
    ret = doipHandleVIDRequestWithVIN(RxPduId, msg, nack);
    break;
  case DOIP_VAN_MSG_OR_VIN_RESPONCE:*/
    /* @SWS_DoIP_00293 */
    /*ret = DOIP_E_NOT_OK_SILENT;
    break;
  case DOIP_ENTITY_STATUS_REQUEST:
    ret = doipHandleEntityStatusRequest(RxPduId, msg, nack);
    break;
  case DOIP_DIAGNOSTIC_POWER_MODE_INFORMATION_REQUEST:
    ret = doipHandleDiagnosticPowerModeInfoRequest(RxPduId, msg, nack);
    break;
  default:*/
    /* @SWS_DoIP_00016 */
    /*nack = DOIP_UNKNOW_PAYLOAD_TYPE_NACK;
    ret = E_NOT_OK;
    break;
  }*/
  return ret;
}

void DoIP_SoAdIfRxIndication(PduIdType RxPduId, const PduInfoType *PduInfoPtr) {
  printf("DoIP_SoAdIfRxIndication\n");
}

void DoIP_SoAdIfTxConfirmation(PduIdType TxPduId, Std_ReturnType result) {
}

void DoIP_MainFunction(void) {
  DoIP_ContextType *context = &DoIP_Context;

  if (DOIP_ACTIVATION_LINE_ACTIVE == context->ActivationLineState) {
    doipHandleInactivityTimer();
    doipHandleAliveCheckResponseTimer();
    doipHandleVehicleAnnouncement();
    doipHandleDiagMsgResponse();
    doipHandleRoutineActivationMain();
  }
}

void DoIP_task(void *pvParameters) {
  for (;;)
  {

    DoIP_MainFunction();
    vTaskDelay(1000);

  } // for
}