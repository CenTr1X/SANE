#ifndef _DOIP_CFG_H
#define _DOIP_CFG_H
/* ================================ [ INCLUDES  ] ============================================== */
/* ================================ [ MACROS    ] ============================================== */
#define DOIP_RX_PID_UDP 0
#define DOIP_RX_PID_TCP0 1
#define DOIP_RX_PID_TCP1 2
#define DOIP_RX_PID_TCP2 3

#define DOIP_MAX_TESTER_CONNECTIONS 3

#define DOIP_MAIN_FUNCTION_PERIOD 10
#define DOIP_CONVERT_MS_TO_MAIN_CYCLES(x) \
  ((x + DOIP_MAIN_FUNCTION_PERIOD - 1) / DOIP_MAIN_FUNCTION_PERIOD)

#define DOIP_P2P_RX 0
#define DOIP_P2P_TX 0
#define DOIP_GW_P2P_RX 1
#define DOIP_GW_P2P_TX 1
#define DOIP_GW_P2A_RX 2
#define DOIP_GW_P2A_TX 2
/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
#endif /* _DOIP_CFG_H */
