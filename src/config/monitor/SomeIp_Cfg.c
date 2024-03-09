#include "SomeIpSd_Cfg.h"
#include "SomeIp_Cfg.h"
#include "SomeIp.h"
#include "SoAd_Cfg.h"
#include "SomeIp_Callback.h"

/* ================================ [ DATAS     ] ============================================== */
static const SomeIp_ServerMethodType someIpServerMethods_server0[] = {
  {
    0x421, /* Method ID */
    0, /* interface version */
    NULL,//SomeIp_server0_method1_OnRequest,
    NULL,//SomeIp_server0_method1_OnFireForgot,
    NULL,//SomeIp_server0_method1_OnAsyncRequest,
    NULL,//SomeIp_server0_method1_OnTpCopyRxData,
    NULL,//SomeIp_server0_method1_OnTpCopyTxData,
    1404 /* resMaxLen */
  },
};

static const SomeIp_ServerEventType someIpServerEvents_server0[] = {
  {
    SD_EVENT_HANDLER_SERVER0_EVENT_GROUP1, /* SD EventGroup Handle ID */
    0xbeef, /* Event ID */
    0, /* interface version */
    NULL,//SomeIp_server0_event_group1_event0_OnTpCopyTxData,
  },
};

static const SomeIp_ClientMethodType someIpClientMethods_client0[] = {
  {
    0x424, /* Method ID */
    0, /* interface version */
    SomeIp_client0_method2_OnResponse,
    NULL,//SomeIp_client0_method2_OnError,
    NULL,//SomeIp_client0_method2_OnTpCopyRxData,
    NULL,//SomeIp_client0_method2_OnTpCopyTxData,
  },
};

static const SomeIp_ClientEventType someIpClientEvents_client0[] = {
  {
    0xabcd, /* Event ID */
    0, /* interface version */
    SomeIp_Hud_OnNotification,//SomeIp_client0_event_group2_event0_OnNotification,
    NULL,//SomeIp_client0_event_group2_event0_OnTpCopyRxData,
  },
};

static SomeIp_ServerContextType someIpServerContext_server0;

static SomeIp_ServerConnectionContextType someIpServerConnectionContext_server0[1];

static const SomeIp_ServerConnectionType someIpServerServiceConnections_server0[1] = {
  {
    &someIpServerConnectionContext_server0[0],
    SOAD_TX_PID_SOMEIP_SERVER0,
    SOAD_SOCKID_SOMEIP_SERVER0,
    NULL
  },
};

static const SomeIp_ServerServiceType someIpServerService_server0 = {
  0x1234, /* serviceId */
  0x4444, /* clientId */
  someIpServerMethods_server0,
  ARRAY_SIZE(someIpServerMethods_server0),
  someIpServerEvents_server0,
  ARRAY_SIZE(someIpServerEvents_server0),
  someIpServerServiceConnections_server0,
  ARRAY_SIZE(someIpServerServiceConnections_server0),
  TCPIP_IPPROTO_UDP,
  &someIpServerContext_server0,
  SOMEIP_CONVERT_MS_TO_MAIN_CYCLES(10),
  NULL,//SomeIp_server0_OnConnect,
};

static SomeIp_ClientServiceContextType someIpClientServiceContext_client0;
static const SomeIp_ClientServiceType someIpClientService_client0 = {
  0xabcd, /* serviceId */
  0x5555, /* clientId */
  SD_CLIENT_SERVICE_HANDLE_ID_CLIENT0, /* sdHandleID */
  someIpClientMethods_client0,
  ARRAY_SIZE(someIpClientMethods_client0),
  someIpClientEvents_client0,
  ARRAY_SIZE(someIpClientEvents_client0),
  &someIpClientServiceContext_client0,
  SOAD_TX_PID_SOMEIP_CLIENT0,
  SomeIp_client0_OnAvailability,
  NULL,
  SOMEIP_CONVERT_MS_TO_MAIN_CYCLES(10),
  SOMEIP_CONVERT_MS_TO_MAIN_CYCLES(1000),
};

static const SomeIp_ServiceType SomeIp_Services[] = {
  {
    TRUE,
    SOAD_SOCKID_SOMEIP_SERVER0,
    &someIpServerService_server0,
  },
  {
    FALSE,
    SOAD_SOCKID_SOMEIP_CLIENT0,
    &someIpClientService_client0,
  },
};

static const uint16_t Sd_PID2ServiceMap[] = {
  SOMEIP_SSID_SERVER0,
  SOMEIP_CSID_CLIENT0,
};

static const uint16_t Sd_PID2ServiceConnectionMap[] = {
  0,
  0,
};

static const uint16_t Sd_TxMethod2ServiceMap[] = {
  SOMEIP_CSID_CLIENT0,/* method2 */ //index of Services
  -1
};

static const uint16_t Sd_TxMethod2PerServiceMap[] = {
  0, /* method2 */
  -1
};

static const uint16_t Sd_TxEvent2ServiceMap[] = {
  SOMEIP_SSID_SERVER0, /* event_group1 event0 */
  -1
};

static const uint16_t Sd_TxEvent2PerServiceMap[] = {
  0, /* event_group1 event0 */
  -1
};

const SomeIp_ConfigType SomeIp_Config = {
  SOMEIP_CONVERT_MS_TO_MAIN_CYCLES(200),
  SomeIp_Services,
  ARRAY_SIZE(SomeIp_Services),
  Sd_PID2ServiceMap,
  Sd_PID2ServiceConnectionMap,
  ARRAY_SIZE(Sd_PID2ServiceMap),
  Sd_TxMethod2ServiceMap,
  Sd_TxMethod2PerServiceMap,
  ARRAY_SIZE(Sd_TxMethod2ServiceMap)-1,
  Sd_TxEvent2ServiceMap,
  Sd_TxEvent2PerServiceMap,
  ARRAY_SIZE(Sd_TxEvent2ServiceMap)-1,
};