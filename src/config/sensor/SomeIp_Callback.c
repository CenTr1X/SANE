#include "SomeIp_Callback.h"
#include "SomeIpSd_Cfg.h"
#include "printf.h"
#include "debug.h"

void SomeIp_client0_OnAvailability(boolean isAvailable) {
  SANE_DEBUGF(SANE_DBG_TRACE,("%s\n", isAvailable?"online":"offline"));
}

Std_ReturnType SomeIp_client0_method2_OnResponse(uint32_t requestId, SomeIp_MessageType *res) {
  SANE_DEBUGF(SANE_DBG_TRACE,("recv reponse with requestid:%x data:%s length:%d\n", requestId, res->data, res->length));
  return E_OK;
}

Std_ReturnType SomeIp_client0_method2_OnError(uint32_t requestId, Std_ReturnType ercd) {
  printf("something wrong!ercd:%d \n");
  return E_OK;
}