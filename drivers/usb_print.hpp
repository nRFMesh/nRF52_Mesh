#ifndef __USB_PRINT_HPP__
#define __USB_PRINT_HPP__


#include <stddef.h>

typedef void (*usb_rx_handler_t)(const char*,uint8_t);

void usb_print_init(usb_rx_handler_t handler);
void usb_print_loop();
void usb_print(const void * p_buf,size_t length);
void usb_printf(const char *format, ...);

class usb_c
{
    public:
        usb_c(usb_rx_handler_t handler);
        void send(const void * p_buf,size_t length);
        void printf(const char *format, ...);
        void loop();

};


#endif /*__USB_PRINT_HPP__*/
