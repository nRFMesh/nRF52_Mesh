extern "C"
{
    #include "usb_print.h"
}


#include <stdarg.h>
#include <stdio.h>


#include "usb_print.hpp"



usb_c::usb_c(usb_rx_handler_t handler)
{
    usb_print_init(handler);
}

void usb_c::send(const void * p_buf,size_t length)
{
    usb_print(p_buf,length);
}

char usb_print_buffer_cpp[256];

void usb_c::printf(const char *format, ...)
{
    va_list args;
    va_start (args, format);
    size_t length = vsnprintf(usb_print_buffer_cpp,256,format, args);
    va_end (args);

    usb_print(usb_print_buffer_cpp, length);
}

void usb_c::loop()
{
    usb_print_loop();
}
