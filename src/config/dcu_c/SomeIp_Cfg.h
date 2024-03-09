#ifndef _SOMEIP_CFG_H
#define _SOMEIP_CFG_H
/* ================================ [ MACROS    ] ============================================== */

#define SOMEIP_MAIN_FUNCTION_PERIOD 10

#define SOMEIP_RX_METHOD_SERVER0_METHOD1 0

#define SOMEIP_TX_METHOD_CLIENT0_METHOD2 0 //TxMethodId

#define SOMEIP_CONVERT_MS_TO_MAIN_CYCLES(x) \
  ((x + SOMEIP_MAIN_FUNCTION_PERIOD - 1) / SOMEIP_MAIN_FUNCTION_PERIOD)
#endif 