#ifndef __USER_UICR_DEFINES__
#define __USER_UICR_DEFINES__

#define UICR_NODE_ID         NRF_UICR->CUSTOMER[0]
#define UICR_RF_CHANNEL      NRF_UICR->CUSTOMER[1]
#define UICR_SLEEP_SEC       NRF_UICR->CUSTOMER[2]
#define UICR_is_listening() (NRF_UICR->CUSTOMER[3] == 0xBABA)
#define UICR_is_router()    (NRF_UICR->CUSTOMER[4] == 0xBABA)
#define UICR_is_rf_cmd()    (NRF_UICR->CUSTOMER[5] == 0xBABA)
#define UICR_is_rf2uart()   (NRF_UICR->CUSTOMER[6] == 0xBABA)
#define UICR_is_uart_cmd()  (NRF_UICR->CUSTOMER[7] == 0xBABA)
#define UICR_RTX_Timeout     NRF_UICR->CUSTOMER[8]
#define UICR_RTX_Max_Timeout NRF_UICR->CUSTOMER[9]
#define UICR_RTX_Count       NRF_UICR->CUSTOMER[10]


#endif /*__USER_UICR_DEFINES__*/