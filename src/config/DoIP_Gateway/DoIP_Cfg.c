/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2021 Parai Wang <parai@foxmail.com>
 *
 * Generated at Fri Mar 10 19:27:44 2023
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include "DoIP.h"
#include "DoIP_Cfg.h"
#include "SoAd_Cfg.h"
#include "DoIP_Callback.h"
/* ================================ [ MACROS    ] ============================================== */
#define DOIP_INITIAL_INACTIVITY_TIME 5000
#define DOIP_GENERAL_INACTIVITY_TIME 5000
#define DOIP_ALIVE_CHECK_RESPONSE_TIMEOUT 50

#define DOIP_TID_P2P 0
#define DOIP_TID_GW_P2P 1
#define DOIP_TID_GW_P2A 2

#define DOIP_RID_default 0

/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/*
extern Std_ReturnType DoIP_default_RoutingActivationAuthenticationCallback(
  boolean *Authentified, const uint8_t *AuthenticationReqData, uint8_t *AuthenticationResData);
extern Std_ReturnType DoIP_default_RoutingActivationConfirmationCallback(
  boolean *Confirmed, const uint8_t *ConfirmationReqData, uint8_t *ConfirmationResData);
static const DoIP_TesterType DoIP_Testers[];
extern Std_ReturnType DoIP_UserGetEID(uint8_t *Data);
extern Std_ReturnType DoIP_UserGetGID(uint8_t *Data);
extern Std_ReturnType DoIP_UserGetPowerModeStatus(uint8_t *PowerState);*/
/* ================================ [ DATAS     ] ============================================== */
static const DoIP_TargetAddressType DoIp_TargetAddress[] = {
  {
    0xdead, /* TargetAddress */
    DOIP_P2P_RX, /* RxPduId */
    DOIP_P2P_TX, /* TxPduId */
  },
  {
    0xcaaa, /* TargetAddress */
    DOIP_GW_P2P_RX, /* RxPduId */
    DOIP_GW_P2P_TX, /* TxPduId */
  },
  {
    0xcaab, /* TargetAddress */
    DOIP_GW_P2A_RX, /* RxPduId */
    DOIP_GW_P2A_TX, /* TxPduId */
  },
};

static const DoIP_TargetAddressType *const DoIP_default_TargetAddressRefs[] = {
  &DoIp_TargetAddress[DOIP_TID_P2P],
  &DoIp_TargetAddress[DOIP_TID_GW_P2P],
  &DoIp_TargetAddress[DOIP_TID_GW_P2A],
};

static const DoIP_RoutingActivationType DoIP_RoutingActivations[] = {
  {
    0xda, /* default */
    0, /* OEMReqLen */
    0, /* OEMResLen */
    DoIP_default_TargetAddressRefs, /* TargetAddressRefs */
    ARRAY_SIZE(DoIP_default_TargetAddressRefs),
    DoIP_default_RoutingActivationAuthenticationCallback,
    DoIP_default_RoutingActivationConfirmationCallback,
  },
};

static const DoIP_RoutingActivationType * const DoIP_default_RoutingActivationRefs[] = {
  &DoIP_RoutingActivations[DOIP_RID_default],
};

static const DoIP_TesterType DoIP_Testers[] = {
  {
    8, /* NumByteDiagAckNack */
    0xbeef, /* TesterSA */
    DoIP_default_RoutingActivationRefs,
    ARRAY_SIZE(DoIP_default_RoutingActivationRefs),
  },
};

static DoIP_TesterConnectionContextType DoIP_TesterConnectionContext[DOIP_MAX_TESTER_CONNECTIONS];

static const DoIP_TesterConnectionType DoIP_TesterConnections[DOIP_MAX_TESTER_CONNECTIONS] = {
  {
    &DoIP_TesterConnectionContext[0],
    SOAD_SOCKID_DOIP_TCP_APT0,
    SOAD_TX_PID_DOIP_TCP_APT0,
  },
  {
    &DoIP_TesterConnectionContext[1],
    SOAD_SOCKID_DOIP_TCP_APT1,
    SOAD_TX_PID_DOIP_TCP_APT1,
  },
  {
    &DoIP_TesterConnectionContext[2],
    SOAD_SOCKID_DOIP_TCP_APT2,
    SOAD_TX_PID_DOIP_TCP_APT2,
  },
};

static const DoIP_TcpConnectionType DoIP_TcpConnections[] = {
  {
    SOAD_SOCKID_DOIP_TCP_SERVER, TRUE, /* RequestAddressAssignment */
  },
};

static const DoIP_UdpConnectionType DoIP_UdpConnections[] = {
  {
    SOAD_SOCKID_DOIP_UDP, SOAD_TX_PID_DOIP_UDP, TRUE, /* RequestAddressAssignment */
  },
};

static DoIP_UdpVehicleAnnouncementConnectionContextType UdpVehicleAnnouncementConnectionContexts[1];

static const DoIP_UdpVehicleAnnouncementConnectionType DoIP_UdpVehicleAnnouncementConnections[] = {
  {
    &UdpVehicleAnnouncementConnectionContexts[0], /* context */
    SOAD_SOCKID_DOIP_UDP,                         /* SoConId */
    SOAD_TX_PID_DOIP_UDP,                         /* SoAdTxPdu */
    FALSE,                                        /* RequestAddressAssignment */
  },
};

static const uint16_t RxPduIdToConnectionMap[] = {
  DOIP_INVALID_INDEX, /* DOIP_RX_PID_UDP */
  0,                  /* DOIP_RX_PID_TCP0 */
  1,                  /* DOIP_RX_PID_TCP1 */
  2,                  /* DOIP_RX_PID_TCP2 */
};

const DoIP_ConfigType DoIP_Config = {
  DOIP_CONVERT_MS_TO_MAIN_CYCLES(DOIP_INITIAL_INACTIVITY_TIME),
  DOIP_CONVERT_MS_TO_MAIN_CYCLES(DOIP_GENERAL_INACTIVITY_TIME),
  DOIP_CONVERT_MS_TO_MAIN_CYCLES(DOIP_ALIVE_CHECK_RESPONSE_TIMEOUT),
  0xFF, /* VinInvalidityPattern */
  DOIP_CONVERT_MS_TO_MAIN_CYCLES(50),  /* InitialVehicleAnnouncementTime */
  DOIP_CONVERT_MS_TO_MAIN_CYCLES(200), /* VehicleAnnouncementInterval */
  3,                                   /* VehicleAnnouncementCount */
  DOIP_GATEWAY, /* NodeType */
  TRUE, /* EntityStatusMaxByteFieldUse */
  DoIP_GetVin,
  DoIP_UserGetEID,
  DoIP_UserGetGID,
  NULL,//DoIP_UserGetPowerModeStatus,
  0xdead, /* LogicalAddress */ 
  DoIp_TargetAddress,
  ARRAY_SIZE(DoIp_TargetAddress),
  DoIP_TcpConnections,
  ARRAY_SIZE(DoIP_TcpConnections),
  DoIP_UdpConnections,
  ARRAY_SIZE(DoIP_UdpConnections),
  DoIP_UdpVehicleAnnouncementConnections,
  ARRAY_SIZE(DoIP_UdpVehicleAnnouncementConnections),
  DoIP_TesterConnections,
  ARRAY_SIZE(DoIP_TesterConnections),
  DoIP_RoutingActivations,
  ARRAY_SIZE(DoIP_RoutingActivations),
  DoIP_Testers,
  ARRAY_SIZE(DoIP_Testers),
  RxPduIdToConnectionMap,
  ARRAY_SIZE(RxPduIdToConnectionMap),
};
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
