#include "SomeIp_Callback.h"
#include "SomeIpSd_Cfg.h"
#include "printf.h"
#include "debug.h"
#include "task.h"

void SomeIp_client0_OnAvailability(boolean isAvailable) {
  SANE_DEBUGF(SANE_DBG_TRACE,("%s\n", isAvailable?"online":"offline"));
}

Std_ReturnType SomeIp_client0_method2_OnResponse(uint32_t requestId, SomeIp_MessageType *res) {
  SANE_DEBUGF(SANE_DBG_TRACE,("recv reponse with requestid:%x data:%s length:%d\n", requestId, res->data, res->length));
  return E_OK;
}

Std_ReturnType SomeIp_Hud_OnNotification(uint32_t requestId, SomeIp_MessageType *evt) {
  //printf("get notification! content:%s length:%d\n", evt->data, evt->length);
  // printf("get notification! content length:%d\n", evt->length);
  // printf("level:%d location:%s description:%s\n", *(evt->data), evt->data+1, evt->data+1 + strlen(evt->data+1) + 1);
  vPortEnterCritical();
  printf("srecv no.%d now : %d \n", *(int*)(evt->data), xTaskGetTickCount());
  vPortExitCritical();
}