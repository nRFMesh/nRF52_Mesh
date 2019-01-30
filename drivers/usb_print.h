#ifndef __USB_PRINT_H__
#define __USB_PRINT_H__


#include <stddef.h>
#include <stdint.h>

typedef void (*usb_rx_handler_t)(const char*,uint8_t);

void usb_print_init(usb_rx_handler_t handler);
void usb_print_loop();
void usb_print(const void * p_buf,size_t length);
void usb_printf(const char *format, ...);

#endif /*__USB_PRINT_H__*/
