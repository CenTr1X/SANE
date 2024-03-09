#ifndef _DOIP_CALLBACK_H
#define _DOIP_CALLBACK_H
/* ================================ [ INCLUDES  ] ============================================== */
#include "Std_Types.h"
#include "ComStack_Types.h"
#include <string.h>

/* ================================ [ MACROS    ] ============================================== */

/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
Std_ReturnType DoIP_GetVin(uint8_t *Data);
Std_ReturnType DoIP_UserGetEID(uint8_t *Data);
Std_ReturnType DoIP_UserGetGID(uint8_t *Data);
Std_ReturnType DoIP_default_RoutingActivationConfirmationCallback(
  boolean *Confirmed, const uint8_t *ConfirmationReqData, uint8_t *ConfirmationResData);
Std_ReturnType DoIP_default_RoutingActivationAuthenticationCallback(
  boolean *Authentified, const uint8_t *AuthenticationReqData, uint8_t *AuthenticationResData);
#endif 