#ifndef _SOMEIP_CALLBACK_H
#define _SOMEIP_CALLBACK_H
#include "Std_Types.h"
#include "TcpIp.h"
#include "SomeIp.h"

void SomeIp_Server_Onsubscribe(boolean isSubscribe, TcpIp_SockAddrType* RemoteAddr);
void SomeIp_server0_OnAvailability(boolean isAvailable);
void SomeIp_client0_OnConnect(uint16_t condId, boolean isConnected);
Std_ReturnType SomeIp_client0_method2_OnRequest(uint32_t requestId, SomeIp_MessageType *req,
                                                  SomeIp_MessageType *res);
Std_ReturnType SomeIp_Sensor_OnFire(uint32_t requestId, SomeIp_MessageType *req);
#endif 