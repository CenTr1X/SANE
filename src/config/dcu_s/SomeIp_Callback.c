#include "SomeIp_Callback.h"
#include "SomeIpSd_Cfg.h"
#include "SomeIp.h"
#include "printf.h"
#include "print.h"
#include "handler_app.h"
#include <string.h>
#include <stdlib.h>


void SomeIp_Server_Onsubscribe(boolean isSubscribe, TcpIp_SockAddrType* RemoteAddr)
{
  onSubscribe_common(SD_EVENT_HANDLER_CLIENT0_EVENT_GROUP2, RemoteAddr);
}

void SomeIp_server0_OnAvailability(boolean isAvailable) {
  printf("%s\n", isAvailable?"online":"offline");
}

void SomeIp_client0_OnConnect(uint16_t condId, boolean isConnected) {
  printf("condId:%d %s\n", condId, (isConnected ? "connected" : "not connected"));
}

//copy
Std_ReturnType SomeIp_client0_method2_OnRequest(uint32_t requestId, SomeIp_MessageType *req,
                                                  SomeIp_MessageType *res)
{
  printf("recv someip msg on method 2, requestid:%d, data:%d, content_length:%d\n", requestId,req->data, req->length);
  memcpy(res->data, req->data, req->length);
  res->length = req->length;
  return E_OK;
  //todo:reply
}

Std_ReturnType SomeIp_Sensor_OnFire(uint32_t requestId, SomeIp_MessageType *req)
{
  //printf("recv sensor fire msg , requestid:%d, data:%d, content_length:%d\n", requestId,req->data, req->length);
  vPortEnterCritical();
  printf("srecvf no.%d now : %d \n", *(int*)(req->data), xTaskGetTickCount());
  vPortExitCritical();
  return E_OK;
}