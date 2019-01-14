#ifndef __USB_PRINT_H__
#define __USB_PRINT_H__


#include <stddef.h>
#include <stdbool.h>

typedef void (*usb_rx_handler_t)(const char);

bool usb_print_enabled();
void usb_print_enable(bool enable);
void usb_print_init(usb_rx_handler_t handler);
void usb_print_loop();
void usb_print(const void * p_buf,size_t length);
void usb_printf(const char *format, ...);

#endif /*__USB_PRINT_H__*/
