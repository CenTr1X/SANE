/* ================================ [ INCLUDES  ] ============================================== */
#include "SoAd.h"
#include "SoAd_Cfg.h"
#include "DoIP.h"
#include "DoIP_Cfg.h"
/* ================================ [ MACROS    ] ============================================== */
/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
static const SoAd_IfInterfaceType SoAd_DoIP_IF = {
  DoIP_SoAdIfRxIndication,
  NULL,
  DoIP_SoAdIfTxConfirmation,
};

static const SoAd_TpInterfaceType SoAd_DoIP_TP_IF = {
  DoIP_SoAdTpStartOfReception,
  DoIP_SoAdTpCopyRxData,
  NULL,
  NULL,
  NULL,
};

static const SoAd_SocketConnectionType SoAd_SocketConnections[] = {
  {
    DOIP_RX_PID_UDP, /* RxPduId */
    SOAD_SOCKID_DOIP_UDP, /* SoConId */
    0, /* GID */
    SOAD_SOCON_UDP_SERVER, /* SoConType */
  },
  {
    -1, /* RxPduId */
    SOAD_SOCKID_DOIP_TCP_SERVER, /* SoConId */
    1, /* GID */
    SOAD_SOCON_TCP_SERVER, /* SoConType */
  },
  {
    DOIP_RX_PID_TCP0, /* RxPduId */
    SOAD_SOCKID_DOIP_TCP_APT0, /* SoConId */
    1, /* GID */
    SOAD_SOCON_TCP_ACCEPT, /* SoConType */
  },
  {
    DOIP_RX_PID_TCP1, /* RxPduId */
    SOAD_SOCKID_DOIP_TCP_APT1, /* SoConId */
    1, /* GID */
    SOAD_SOCON_TCP_ACCEPT, /* SoConType */
  },
  {
    DOIP_RX_PID_TCP2, /* RxPduId */
    SOAD_SOCKID_DOIP_TCP_APT2, /* SoConId */
    1, /* GID */
    SOAD_SOCON_TCP_ACCEPT, /* SoConType */
  }
};

static SoAd_SocketContextType SoAd_SocketContexts[ARRAY_SIZE(SoAd_SocketConnections)];

static const SoAd_SocketConnectionGroupType SoAd_SocketConnectionGroups[] = {
  {
    /* 0: DOIP_UDP */
    &SoAd_DoIP_IF, /* Interface */
    DoIP_SoConModeChg, /* SoConModeChgNotification */
    TCPIP_IPPROTO_UDP, /* ProtocolType */
    TCPIP_IPV4_ADDR(224,244,224,245), /* Remote */
    -1, /* SoConId */
    13400, /* Port */
    0, /* LocalAddrId */
    1, /* numOfConnections */
    FALSE, /* AutomaticSoConSetup */
    FALSE, /* IsTP */
    TRUE, /* IsMulitcast */
  },
  {
    /* 1: DOIP_TCP */
    &SoAd_DoIP_TP_IF, /* Interface */
    DoIP_SoConModeChg, /* SoConModeChgNotification */
    TCPIP_IPPROTO_TCP, /* ProtocolType */
    0, /* Remote */
    SOAD_SOCKID_DOIP_TCP_APT0, /* SoConId */
    13400, /* Port */
    TCPIP_LOCALADDRID_ANY, /* LocalAddrId */
    3, /* numOfConnections */
    FALSE, /* AutomaticSoConSetup */
    TRUE, /* IsTP */
    FALSE, /* IsMulitcast */
  },
};

static const SoAd_SoConIdType TxPduIdToSoCondIdMap[] = {
  SOAD_SOCKID_DOIP_UDP, /* SOAD_TX_PID_DOIP_UDP */
  SOAD_SOCKID_DOIP_TCP_APT0, /* SOAD_TX_PID_DOIP_TCP_APT0 */
  SOAD_SOCKID_DOIP_TCP_APT1, /* SOAD_TX_PID_DOIP_TCP_APT1 */
  SOAD_SOCKID_DOIP_TCP_APT2, /* SOAD_TX_PID_DOIP_TCP_APT2 */
};

const SoAd_ConfigType SoAd_Config = {
  SoAd_SocketConnections,
  SoAd_SocketContexts,
  ARRAY_SIZE(SoAd_SocketConnections),
  TxPduIdToSoCondIdMap,
  ARRAY_SIZE(TxPduIdToSoCondIdMap),
  SoAd_SocketConnectionGroups,
  ARRAY_SIZE(SoAd_SocketConnectionGroups),
};
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
