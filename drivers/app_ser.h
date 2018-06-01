#ifndef __APP_SER_H__
#define __APP_SER_H__


#include <stdint.h>

typedef void (*app_serial_handler_t)(const char*,uint8_t);
typedef void (*app_serial_tx_handler_t)(void);


void ser_init(app_serial_handler_t handler,app_serial_tx_handler_t tx_handler);
void ser_send(char* message);

#endif /*__APP_SER_H__*/
