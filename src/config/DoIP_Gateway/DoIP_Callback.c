#include "DoIP_Callback.h"

Std_ReturnType DoIP_GetVin(uint8_t *Data){
  static const char *vin = "VIN20210822-PARAI";
  memcpy(Data, vin, 17);
  return E_OK;
}

Std_ReturnType DoIP_UserGetEID(uint8_t *Data) {
  static const char *EID = "EID123";
  memcpy(Data, EID, 6);
  return E_OK;
}

Std_ReturnType DoIP_UserGetGID(uint8_t *Data) {
  static const char *GID = "GID123";
  memcpy(Data, GID, 6);
  return E_OK;
}

Std_ReturnType DoIP_default_RoutingActivationAuthenticationCallback(
  boolean *Authentified, const uint8_t *AuthenticationReqData, uint8_t *AuthenticationResData) {
  *Authentified = TRUE;
  return E_OK;
}

Std_ReturnType DoIP_default_RoutingActivationConfirmationCallback(
  boolean *Confirmed, const uint8_t *ConfirmationReqData, uint8_t *ConfirmationResData) {
  printf("DOIP default activated\n");
  *Confirmed = TRUE;
  return E_OK;
}