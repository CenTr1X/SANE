#ifndef _DOIP_TESTER_H
#define _DOIP_TESTER_H
/* ================================ [ INCLUDES  ] ============================================== */
#include "Std_Types.h"
#include "DoIP.h"
#include "sys/queue.h"
#include "TcpIp.h"
/* ================================ [ MACROS    ] ============================================== */
#define DOIP_PROTOCOL_VERSION 2
#define DOIP_HEADER_LENGTH 8u

/* @SWS_DoIP_00008, @SWS_DoIP_00009 */
#define DOIP_GENERAL_HEADER_NEGATIVE_ACK 0x0000u                /* UDP/TCP*/
#define DOIP_VID_REQUEST 0x0001u                                /* UDP */
#define DOIP_VID_REQUEST_WITH_EID 0x0002u                       /* UDP */
#define DOIP_VID_REQUEST_WITH_VIN 0x0003u                       /* UDP */
#define DOIP_VAN_MSG_OR_VIN_RESPONCE 0x0004u                    /* UDP */
#define DOIP_ROUTING_ACTIVATION_REQUEST 0x0005u                 /* TCP */
#define DOIP_ROUTING_ACTIVATION_RESPONSE 0x0006u                /* TCP */
#define DOIP_ALIVE_CHECK_REQUEST 0x0007u                        /* TCP */
#define DOIP_ALIVE_CHECK_RESPONSE 0x0008u                       /* TCP */
#define DOIP_ENTITY_STATUS_REQUEST 0x4001u                      /* UDP */
#define DOIP_ENTITY_STATUS_RESPONSE 0x4002u                     /* UDP */
#define DOIP_DIAGNOSTIC_POWER_MODE_INFORMATION_REQUEST 0x4003u  /* UDP */
#define DOIP_DIAGNOSTIC_POWER_MODE_INFORMATION_RESPONSE 0x4004u /* UDP */
#define DOIP_DIAGNOSTIC_MESSAGE 0x8001                          /* TCP */
#define DOIP_DIAGNOSTIC_MESSAGE_POSITIVE_ACK 0x8002             /* TCP */
#define DOIP_DIAGNOSTIC_MESSAGE_NEGATIVE_ACK 0x8003             /* TCP */

/* ================================ [ TYPES     ] ============================================== */
typedef struct {
  uint8_t VIN[17];
  uint16_t LA;
  uint8_t EID[6];
  uint8_t GID[6];
} doip_node_t;

struct doip_node_s {
  doip_node_t node;
  uint8_t FA;
  uint8_t status;
  uint16_t SA;
  uint16_t LA;
  TcpIp_SockAddrType RemoteAddr;
  TcpIp_SocketIdType sock;
  bool connected;
  bool activated;
  //pthread_t thread;
  //pthread_mutex_t lock;
  //sem_t sem;
  //Std_TimerType alive_request_timer;
  //Std_TimerType alive_timer;
  struct {
    int result;
    uint16_t payload_type;
  } R; /* for response */
  bool stopped;
  struct doip_client_s *client;
  STAILQ_ENTRY(doip_node_s) entry;
  uint8_t buffer[4096];
};

typedef struct doip_client_s {
  TcpIp_SocketIdType discovery;
  TcpIp_SocketIdType test_equipment_request;
  char ip[32];
  int port;
  //pthread_t thread;
  //pthread_mutex_t lock;
  bool stopped;
  //sem_t sem;
  struct {
    int result;
    uint16_t payload_type;
  } R; /* for response */
  STAILQ_HEAD(, doip_node_s) nodes;
  uint32_t numOfNodes;
  uint8_t buffer[4096];
} doip_client_t;
/* ================================ [ DECLARES  ] ============================================== */
void doip_tester();

static void doip_handle_udp_message(doip_client_t *client, TcpIp_SockAddrType *RemoteAddr,
                                    uint8_t *data, uint16_t length);
#endif /* _DOIP_TESTER_H */