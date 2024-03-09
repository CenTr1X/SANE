/* ================================ [ INCLUDES  ] ============================================== */
#include "SoAd.h"
#include "SomeIpSd.h"
#include "SoAd_Cfg.h"
#include "SomeIpSd_Cfg.h"
#include "SomeIp.h"
/* ================================ [ DATAS     ] ============================================== */
static const SoAd_IfInterfaceType SoAd_SD_IF = {
  Sd_RxIndication,//Sd_RxIndication,
  NULL,
  NULL,
};

static const SoAd_IfInterfaceType SoAd_SOMEIP_IF = {
  SomeIp_RxIndication,
  NULL,
  NULL,
};

static const SoAd_SocketConnectionType SoAd_SocketConnections[] = {
  {
    SD_RX_PID_MULTICAST, /* RxPduId */
    SOAD_SOCKID_SD_MULTICAST, /* SoConId */
    0, /* GID */
    SOAD_SOCON_UDP_SERVER, /* SoConType */
  },
  {
    SD_RX_PID_UNICAST, /* RxPduId */
    SOAD_SOCKID_SD_UNICAST, /* SoConId */
    1, /* GID */
    SOAD_SOCON_UDP_SERVER, /* SoConType */
  },
  {
    SOMEIP_RX_PID_SOMEIP_CLIENT0, /* RxPduId */
    SOAD_SOCKID_SOMEIP_CLIENT0, /* SoConId */
    2, /* GID */
    SOAD_SOCON_UDP_SERVER, /* SoConType */
  },
  {
    SOMEIP_RX_PID_SOMEIP_SERVER0, /* RxPduId */
    SOAD_SOCKID_SOMEIP_SERVER0, /* SoConId */
    3, /* GID */
    SOAD_SOCON_UDP_CLIENT, /* SoConType */
  },
};


static SoAd_SocketContextType SoAd_SocketContexts[ARRAY_SIZE(SoAd_SocketConnections)];

static const SoAd_SocketConnectionGroupType SoAd_SocketConnectionGroups[] = {
  {
    /* 0: SD_MULTICAST */
    &SoAd_SD_IF, /* Interface */
    NULL,//Sd_SoConModeChg, /* SoConModeChgNotification */
    TCPIP_IPPROTO_UDP, /* ProtocolType */
    TCPIP_IPV4_ADDR(224,244,224,245),// /* Remote */
    -1, /* SoConId */
    30490, /* Port */
    0, /* LocalAddrId */
    3, /* numOfConnections */
    FALSE, /* AutomaticSoConSetup */
    FALSE, /* IsTP */
    TRUE, /* IsMulitcast */
  },
  {
    /* 1: SD_UNICAST */
    &SoAd_SD_IF, /* Interface */
    NULL,//Sd_SoConModeChg, /* SoConModeChgNotification */
    TCPIP_IPPROTO_UDP, /* ProtocolType */
    0, /* Remote */
    -1, /* SoConId */
    30500, /* Port */
    TCPIP_LOCALADDRID_ANY, /* LocalAddrId */
    3, /* numOfConnections */
    FALSE, /* AutomaticSoConSetup */
    FALSE, /* IsTP */
    FALSE, /* IsMulitcast */
  },
  {
    /* 2: SOMEIP_CLIENT0 */
    &SoAd_SOMEIP_IF, /* Interface */
    SomeIp_SoConModeChg, /* SoConModeChgNotification */
    TCPIP_IPPROTO_UDP, /* ProtocolType */
    0, /* Remote */
    -1, /* SoConId */
    30568, /* Port */
    TCPIP_LOCALADDRID_ANY, /* LocalAddrId */
    1, /* numOfConnections */
    FALSE, /* AutomaticSoConSetup */
    FALSE, /* IsTP */
    FALSE, /* IsMulitcast */
  },
  {
    /* 3: SOMEIP_SERVER0 */
    &SoAd_SOMEIP_IF, /* Interface */
    SomeIp_SoConModeChg, /* SoConModeChgNotification */
    TCPIP_IPPROTO_UDP, /* ProtocolType */
    0, /* Remote */
    -1, /* SoConId */
    60000, /* Port */
    TCPIP_LOCALADDRID_ANY, /* LocalAddrId */
    1, /* numOfConnections */
    FALSE, /* AutomaticSoConSetup */
    FALSE, /* IsTP */
    FALSE, /* IsMulitcast */
  },
};

static const SoAd_SoConIdType TxPduIdToSoCondIdMap[] = {
  SOAD_SOCKID_SD_MULTICAST, /* SOAD_TX_PID_SD_MULTICAST */
  SOAD_SOCKID_SD_UNICAST, /* SOAD_TX_PID_SD_UNICAST */
  SOAD_SOCKID_SOMEIP_CLIENT0, /* SOAD_TX_PID_SOMEIP_CLIENT0 */
  SOAD_SOCKID_SOMEIP_SERVER0, /* SOAD_TX_PID_SOMEIP_SERVER0 */
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