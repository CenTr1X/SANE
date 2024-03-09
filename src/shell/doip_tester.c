#include "doip_tester.h"
#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"
#include "string.h"
#include <stdlib.h>
#include <ctype.h>

/* ================================ [ MACROS    ] ============================================== */
#define DOIP_E_OK 0
#define DOIP_E_NOT_OK -1
#define DOIP_E_NEGATIVE -2
#define DOIP_E_INVAL -3
#define DOIP_E_TOO_LONG -4
#define DOIP_E_TIMEOUT -5
#define DOIP_E_AGAIN -6
#define DOIP_E_NODEV -7
#define DOIP_E_ACCES -8
#define DOIP_E_NOSPC -9
#define DOIP_E_NOMEM -10

#define DOIP_E_OK_SILENT -999
/* ================================ [ FUNCTIONS ] ============================================== */
static void vlog(const char *prefix, uint8_t *data, int length) {
  int i;
  printf(prefix);
  for (i = 0; i < length; i++) {
    printf("%02X ", data[i]);
  }
  printf("\t");
  for (i = 0; i < length; i++) {
    if (isprint(data[i])) {
      printf("%c", data[i]);
    } else {
      printf(".");
    }
  }
  printf("\n");
}

static void doip_fill_header(uint8_t *header, uint16_t payloadType, uint32_t payloadLength) {
  header[0] = DOIP_PROTOCOL_VERSION;
  header[1] = ~DOIP_PROTOCOL_VERSION;
  header[2] = (payloadType >> 8) & 0xFF;
  header[3] = payloadType & 0xFF;
  header[4] = (payloadLength >> 24) & 0xFF;
  header[5] = (payloadLength >> 16) & 0xFF;
  header[6] = (payloadLength >> 8) & 0xFF;
  header[7] = payloadLength & 0xFF;
}

static void doip_add_node(doip_client_t *client, struct doip_node_s *node) {
  bool exist = false;
  struct doip_node_s *n;
  STAILQ_FOREACH(n, &client->nodes, entry) {
    if ((0 == memcmp(n->node.VIN, node->node.VIN, 17)) &&
        (0 == memcmp(n->node.EID, node->node.EID, 6)) &&
        (0 == memcmp(n->node.GID, node->node.GID, 6)) && (n->node.LA == node->node.LA)) {
      printf("node exsits, and will be free\n");
      exist = true;
      break;
    }
  }
  if (false == exist) {
    node->connected = false;
    node->activated = false;
    node->stopped = false;
    node->client = client;
    STAILQ_INSERT_TAIL(&client->nodes, node, entry);
    client->numOfNodes++;
    printf("discovery node %d.%d.%d.%d:%d online\n", node->RemoteAddr.addr[0],
                  node->RemoteAddr.addr[1], node->RemoteAddr.addr[2], node->RemoteAddr.addr[3],
                  node->RemoteAddr.port);
  } else {
    printf("node free\n");
    free(node);
    printf("free done\n");
  }
}

static int doip_handle_VAN_VIN_response(doip_client_t *client, TcpIp_SockAddrType *RemoteAddr,
                                        uint8_t *payload, uint32_t length) {
  struct doip_node_s *node;
  int r = 0;

  printf("VAN message with length=%d\n", length);
  if (33 == length) {
    node = (struct doip_node_s *)malloc(sizeof(*node));
    if (NULL != node) {
      memcpy(node->node.VIN, &payload[0], 17);
      node->node.LA = ((uint16_t)payload[17] << 8) + payload[18];
      memcpy(node->node.EID, &payload[19], 6);
      memcpy(node->node.GID, &payload[25], 6);
      node->FA = payload[31];
      node->status = payload[32];
      node->RemoteAddr = *RemoteAddr;
      doip_add_node(client, node);
    } else {
      printf("OoM\n");
      r = DOIP_E_NOMEM;
    }
  } else {
    r = DOIP_E_INVAL;
  }

  return r;
}


static void doip_handle_udp_message(doip_client_t *client, TcpIp_SockAddrType *RemoteAddr,
                                    uint8_t *data, uint16_t length) {
  uint16_t payloadType = (uint16_t)-1;
  uint32_t payloadLength;
  int r = 0;

  printf("udp message from %d.%d.%d.%d:%d, length=%d\n", RemoteAddr->addr[0], RemoteAddr->addr[1],
         RemoteAddr->addr[2], RemoteAddr->addr[3], RemoteAddr->port, length);

  if ((length >= DOIP_HEADER_LENGTH) && (DOIP_PROTOCOL_VERSION == data[0]) &&
      (data[0] = ((~data[1])) & 0xFF)) {
    payloadType = ((uint16_t)data[2] << 8) + data[3];
    payloadLength =
      ((uint32_t)data[4] << 24) + ((uint32_t)data[5] << 16) + ((uint32_t)data[6] << 8) + data[7];
    if ((payloadLength + DOIP_HEADER_LENGTH) <= length) {
      switch (payloadType) {
      case DOIP_VID_REQUEST:
        r = DOIP_E_OK_SILENT;
        printf("DOIP_E_OK_SILENT\n");
        break;
      case DOIP_VAN_MSG_OR_VIN_RESPONCE:
        printf("DOIP_VAN_MSG_OR_VIN_RESPONCE\n");
        r = doip_handle_VAN_VIN_response(client, RemoteAddr, &data[DOIP_HEADER_LENGTH],
                                         payloadLength);
        break;
      default:
        r = DOIP_E_INVAL;
        printf("unsupported payload type 0x%X\n", payloadType);
        break;
      }
    } else {
      r = DOIP_E_TOO_LONG;
      printf("message too long\n");
    }
  } else {
    r = DOIP_E_INVAL;
    printf("invalid udp message\n");
  }

  if (DOIP_E_OK_SILENT != r) {
    client->R.payload_type = payloadType;
    client->R.result = r;
  }
}

static int doip_handle_activate_response(struct doip_node_s *node, uint8_t *payload,
                                         uint32_t length) {
  int r = 0;
  uint16_t sa, la;
  uint8_t resCode;
  if (13 == length) {
    sa = ((uint16_t)payload[0] << 8) + payload[1];
    la = ((uint16_t)payload[2] << 8) + payload[3];
    resCode = payload[4];
    if (0x10 == resCode) {
      node->SA = sa;
      node->LA = la;
      printf("activated okay with SA=%X, LA=%X\n", sa, la);
    } else {
      printf("activate failed with response code 0x%x\n", resCode);
      r = DOIP_E_NEGATIVE;
      node->stopped = true;
    }
  } else {
    printf("activate response with invalid length\n");
    r = DOIP_E_INVAL;
  }
  return r;
}

static void doip_handle_tcp_response(struct doip_node_s *node, uint8_t *data, uint16_t length) {
  int r = 0;
  uint16_t payloadType = (uint16_t)-1;
  uint32_t payloadLength;
  printf("doip_handle_tcp_response\n");
  if ((length >= DOIP_HEADER_LENGTH) && (DOIP_PROTOCOL_VERSION == data[0]) &&
      (data[0] = ((~data[1])) & 0xFF)) {
    payloadType = ((uint16_t)data[2] << 8) + data[3];
    payloadLength =
      ((uint32_t)data[4] << 24) + ((uint32_t)data[5] << 16) + ((uint32_t)data[6] << 8) + data[7];
    if ((payloadLength + DOIP_HEADER_LENGTH) <= length) {
      switch (payloadType) {
      case DOIP_GENERAL_HEADER_NEGATIVE_ACK:
        printf("DOIP_GENERAL_HEADER_NEGATIVE_ACK\n");
        //r = doip_handle_general_negative_ack(node, &data[DOIP_HEADER_LENGTH], payloadLength);
        break;
      case DOIP_ROUTING_ACTIVATION_RESPONSE:
        printf("DOIP_ROUTING_ACTIVATION_RESPONSE\n");
        r = doip_handle_activate_response(node, &data[DOIP_HEADER_LENGTH], payloadLength);
        break;
      case DOIP_ALIVE_CHECK_REQUEST:
        printf("DOIP_ALIVE_CHECK_REQUEST\n");
        //r = doip_handle_alive_check_request(node, &data[DOIP_HEADER_LENGTH], payloadLength);
        break;
      case DOIP_ALIVE_CHECK_RESPONSE:
        printf("DOIP_ALIVE_CHECK_RESPONSE\n");
        //r = doip_handle_alive_check_response(node, &data[DOIP_HEADER_LENGTH], payloadLength);
        break;
      case DOIP_DIAGNOSTIC_MESSAGE:
        if (payloadLength > 4) {
          r = payloadLength - 4;
        } else {
          r = DOIP_E_INVAL;
        }
        break;
      case DOIP_DIAGNOSTIC_MESSAGE_POSITIVE_ACK:
        r = 0;
        break;
      case DOIP_DIAGNOSTIC_MESSAGE_NEGATIVE_ACK:
        printf("diagnostic negative response code 0x%X\n", data[DOIP_HEADER_LENGTH + 4]);
        r = DOIP_E_NEGATIVE;
        break;
      default:
        r = DOIP_E_INVAL;
        printf("unsupported payload type 0x%X\n", payloadType);
        break;
      }
    } else {
      r = DOIP_E_TOO_LONG;
      printf("message too long\n");
    }
  } else {
    r = DOIP_E_INVAL;
    printf("invalid tcp message\n");
  }

  if (DOIP_E_OK_SILENT != r) {
    node->R.payload_type = payloadType;
    node->R.result = r;
  }
}


static void doip_daemon(void *arg) {
  Std_ReturnType ret;
  doip_client_t *client = (doip_client_t *)arg;
  uint32_t length;
  TcpIp_SockAddrType RemoteAddr;

  printf("DoIP Client request on %s:%d\n", client->ip, client->port);
  while (false == client->stopped) {
    length = sizeof(client->buffer);
    ret = TcpIp_RecvFrom(client->discovery, &RemoteAddr, client->buffer, &length);
    if ((E_OK == ret) && (length > 0)) {
      doip_handle_udp_message(client, &RemoteAddr, client->buffer, length);
    }
    length = sizeof(client->buffer);
    ret = TcpIp_RecvFrom(client->test_equipment_request, &RemoteAddr, client->buffer, &length);
    if ((E_OK == ret) && (length > 0)) {
      doip_handle_udp_message(client, &RemoteAddr, client->buffer, length);
    }
    vTaskDelay(1000);
  }

  printf("DoIP Client offline\n");

  TcpIp_Close(client->discovery, TRUE);

}

static void *node_daemon(void *arg) {
  Std_ReturnType ret;
  struct doip_node_s *node = (struct doip_node_s *)arg;
  uint32_t length;
  printf("DoIP node online\n");
  node->connected = true;
  while (false == node->stopped) {
    length = sizeof(node->buffer);
    printf("try to recv...\n");
    ret = TcpIp_Recv(node->sock, node->buffer, &length);
    if ((E_OK == ret) && (length > 0)) {
      doip_handle_tcp_response(node, node->buffer, length);
    }
    vTaskDelay(1000);
  }

  TcpIp_Close(node->sock, TRUE);
  node->connected = false;
  node->activated = false;

  printf("DoIP node offline\n");
  return NULL;
}

doip_client_t *doip_create_client(const char *ip, int port) {
  doip_client_t *client = NULL;
  TcpIp_SocketIdType discovery;
  TcpIp_SocketIdType test_equipment_request;
  Std_ReturnType ret = E_OK;
  uint16_t u16Port = port;
  TcpIp_SockAddrType ipv4Addr;
  uint32_t ipAddr = TcpIp_InetAddr(ip);


  discovery = TcpIp_Create(TCPIP_IPPROTO_UDP);
  if (discovery >= 0) {
    ret = TcpIp_Bind(discovery, 0, &u16Port);
    if (E_OK != ret) {
      printf("Failed to bind\n");
      TcpIp_Close(discovery, TRUE);
      ret = E_NOT_OK;
    } else {
      TcpIp_SetupAddrFrom(&ipv4Addr, ipAddr, u16Port);
      ret = TcpIp_AddToMulticast(discovery, &ipv4Addr);
      if (E_OK == ret) {
        test_equipment_request = TcpIp_Create(TCPIP_IPPROTO_UDP);
        if (test_equipment_request < 0) {
          TcpIp_Close(discovery, TRUE);
          ret = E_NOT_OK;
        } else {
          TcpIp_Bind(test_equipment_request, 0, &u16Port);
        }
      }
    }
  } else {
    ret = E_NOT_OK;
  }

  if (E_OK == ret) {
    client = malloc(sizeof(doip_client_t));
    if (NULL != client) {
      client->discovery = discovery;
      client->test_equipment_request = test_equipment_request;
      client->stopped = false;
      strcpy(client->ip, ip);
      client->port = port;
      client->numOfNodes = 0;
      STAILQ_INIT(&client->nodes);
    } else {
      printf("Failed to allocate memory\n");
      ret = E_NOT_OK;
    }
  }

  if (E_OK == ret) {
    //pthread_create(&client->thread, NULL, doip_daemon, client);
    xTaskCreate(doip_daemon, "doip_daemon", configMINIMAL_STACK_SIZE * 3, (void*)client, 1, NULL);
  }

  return client;
}


//wait for client->numOfNodes increase

int doip_await_vehicle_announcement(doip_client_t *client, doip_node_t **nodes, int numOfNodes,
                                    uint32_t timeout ) {
  int num = 0;
  int i = 0;
  struct doip_node_s *n;

  //just wait...
  while (0 == client->numOfNodes) {
    printf("wait...\n");
    vTaskDelay(1000);
  }

  printf("waiting finished! client->numOfNodes:%d\n", client->numOfNodes);

  num = client->numOfNodes;
  if (num > numOfNodes) {
    num = numOfNodes;
  }

  if (num > 0) {
    STAILQ_FOREACH(n, &client->nodes, entry) {
      nodes[i] = &n->node;
      i++;
      if (i >= num) {
        break;
      }
    }
  }
  return client->numOfNodes;
}

int doip_connect(doip_node_t *node) {
  int r = 0;
  TcpIp_SocketIdType sockId;
  Std_ReturnType ret = E_OK;
  struct doip_node_s *n = (struct doip_node_s *)node;

  printf("doip connect\n");
  if (n->connected) {
    r = DOIP_E_AGAIN;
  }

  if (0 == r) {
    sockId = TcpIp_Create(TCPIP_IPPROTO_TCP);
    if (sockId >= 0) {
      ret = TcpIp_TcpConnect(sockId, &n->RemoteAddr);
      if (E_OK != ret) {
        printf("Failed to connect\n");
        TcpIp_Close(sockId, TRUE);
        ret = E_NOT_OK;
      }
    } else {
      ret = E_NOT_OK;
    }

    if (E_OK == ret) {
      n->sock = sockId;
      xTaskCreate(node_daemon, "node_daemon", configMINIMAL_STACK_SIZE * 3, (void*)n, 1, NULL);
    } else {
      r = DOIP_E_NODEV;
    }

  }

  return r;
}
/*
static int doip_node_wait(struct doip_node_s *node) {
  int r;
  struct timespec ts;

  node->R.result = DOIP_E_TIMEOUT;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 5;
  pthread_mutex_unlock(&node->lock);
  sem_timedwait(&node->sem, &ts);
  r = node->R.result;
  pthread_mutex_lock(&node->lock);

  return r;
}*/

int doip_activate(doip_node_t *node, uint16_t sa, uint8_t at, uint8_t *oem, uint8_t oem_len) {
  int r = 0;
  Std_ReturnType ret;
  struct doip_node_s *n = (struct doip_node_s *)node;

  printf("doip_activate\n");
  if ((0 == oem_len) || ((4 == oem_len) && (NULL != oem))) {
  } else {
    r = DOIP_E_INVAL;
  }
  if (n->activated) {
    r = DOIP_E_AGAIN;
  }
  if (0 == r) {
    doip_fill_header(n->buffer, DOIP_ROUTING_ACTIVATION_REQUEST, 7 + oem_len);
    n->buffer[DOIP_HEADER_LENGTH] = (sa >> 8) & 0xFF;
    n->buffer[DOIP_HEADER_LENGTH + 1] = sa & 0xFF;
    n->buffer[DOIP_HEADER_LENGTH + 2] = at;
    if (oem_len > 0) {
      memcpy(&n->buffer[DOIP_HEADER_LENGTH + 3], oem, oem_len);
    }
    ret = TcpIp_Send(n->sock, n->buffer, DOIP_HEADER_LENGTH + 7 + oem_len);
    if (E_OK == ret) {
      //r = doip_node_wait(n);
      r = 0;
      while(DOIP_ROUTING_ACTIVATION_RESPONSE != n->R.payload_type)
      {
        printf("waiting response...\n");
        vTaskDelay(500);
      }
      if ((0 == r) && (DOIP_ROUTING_ACTIVATION_RESPONSE == n->R.payload_type)) {
        n->activated = true;
      } else {
        printf("response type:0x%x\n", n->R.payload_type);
        r = DOIP_E_NOT_OK;
      }
    } else {
      r = DOIP_E_ACCES;
    }
  }

  return r;
}

void doip_destory_client(doip_client_t *client) {
  struct doip_node_s *n;
  client->stopped = true;
  while (false == STAILQ_EMPTY(&client->nodes)) {
    n = STAILQ_FIRST(&client->nodes);
    STAILQ_REMOVE_HEAD(&client->nodes, entry);
    n->stopped = true;
    free(n);
  }
  free(client);
}

int doip_transmit(doip_node_t *node, uint16_t ta, const uint8_t *txBuffer, size_t txSize,
                  uint8_t *rxBuffer, size_t rxSize) {
  int r = 0;
  Std_ReturnType ret;
  struct doip_node_s *n = (struct doip_node_s *)node;

  if (false == n->activated) {
    r = DOIP_E_AGAIN;
  }

  if (0 == r) {
    doip_fill_header(n->buffer, DOIP_DIAGNOSTIC_MESSAGE, 4 + txSize);
    n->buffer[DOIP_HEADER_LENGTH] = (n->SA >> 8) & 0xFF;
    n->buffer[DOIP_HEADER_LENGTH + 1] = n->SA & 0xFF;
    n->buffer[DOIP_HEADER_LENGTH + 2] = (ta >> 8) & 0xFF;
    n->buffer[DOIP_HEADER_LENGTH + 3] = ta & 0xFF;
    memcpy(&n->buffer[DOIP_HEADER_LENGTH + 4], txBuffer, txSize);
    ret = TcpIp_Send(n->sock, n->buffer, DOIP_HEADER_LENGTH + 4 + txSize);
    if (E_OK == ret) {
      //r = doip_node_wait(n);
      while((DOIP_DIAGNOSTIC_MESSAGE_POSITIVE_ACK != n->R.payload_type)) {
        printf("waiting for diag ack");
        vTaskDelay(500);
      }
      if ((0 == r) && (DOIP_DIAGNOSTIC_MESSAGE_POSITIVE_ACK == n->R.payload_type)) {
      } else {
        r = DOIP_E_NOT_OK;
      }

      if (0 == r) {
        if (NULL != rxBuffer) {
          //r = doip_node_wait(n);
          while((DOIP_DIAGNOSTIC_MESSAGE != n->R.payload_type)) {
              printf("waiting for diag msg\n");
              vTaskDelay(500);
          }
          r = 1;
          if ((r > 0) && (DOIP_DIAGNOSTIC_MESSAGE == n->R.payload_type)) {
            if (rxSize >= r) {
              memcpy(rxBuffer, &n->buffer[DOIP_HEADER_LENGTH + 4], r);
            } else {
              r = DOIP_E_NOSPC;
            }
          } else {
            r = DOIP_E_NOT_OK;
          }
        }
      }
    }
  }

  return r;
}


void doip_tester() {
  int r = 0;
  int ch;
  char *ip = "224.244.224.245";
  int port = 13400;
  doip_client_t *client;
  doip_node_t *node;
  uint16_t sa = 0xbeef;
  uint8_t at = 0xda;
  uint16_t ta = 0xdead;
  int length = 0;
  uint8_t data[4095];
  char bytes[3] = {0, 0, 0};
  int i;

  length = 8;
  memcpy(data, "test1234", length);
  client = doip_create_client(ip, port);

  if (NULL == client) {
    printf("Failed to clreate doip client <%s:%d>\n", ip, port);
  }

  r = doip_await_vehicle_announcement(client, &node, 1, 3000);
/*not send request actively
  if (r <= 0) {
    node = doip_request(client);
    if (NULL == node) {
      r = -1;
      printf("target is not reachable\n");
    } else {
      r = 0;
    }
  } else {
    r = 0;
  }*/
  r = 0;
  if (0 == r) {
    printf("Found Vehicle:\n");
    vlog("  VIN: ", node->VIN, 17);
    vlog("  EID: ", node->EID, 6);
    vlog("  GID: ", node->GID, 6);
    printf("  LA: %X\n", node->LA);
    r = doip_connect(node);
    if (0 != r) {
      printf("connect failed\n");
    }
  }

  if (0 == r) {
    r = doip_activate(node, sa, at, NULL, 0);
    if (0 != r) {
      printf("activate failed, error:%d\n", r);
    }
  }

  if (0 == r) {
    printf("TX: ");
    for (i = 0; i < length; i++) {
      printf("%02X ", data[i]);
    }
    printf("\n");
    r = doip_transmit(node, ta, data, length, data, sizeof(data));

    if (r > 0) {
      printf("RX: ");
      for (i = 0; i < r; i++) {
        printf("%02X ", data[i]);
      }
      printf("\n");
    } else {
      printf("failed with error %d\n", r);
    }
  }

  doip_destory_client(client);

}