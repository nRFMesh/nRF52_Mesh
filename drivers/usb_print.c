#include <stdarg.h>
#include <stdio.h>

#include "usb_print.h"


#include "nrf.h"
#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"

#include "app_error.h"
#include "app_util.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

#include "boards.h"
#include "bsp.h"

#include "utils.h"
#include "nrf_delay.h"
#include "nrf_ringbuf.h"

#define LED_USB_RESUME      (BSP_BOARD_LED_0)
#define LED_RED             (BSP_BOARD_LED_1)
#define LED_CDC_ACM_RX      (BSP_BOARD_LED_2)
#define LED_CDC_ACM_TX      (BSP_BOARD_LED_3)

#define NRF_LOG_INFO(X,...)

/**
 * @brief Enable power USB detection
 *
 * Configure if example supports USB port connection
 */
#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION true
#endif


static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1


/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

usb_rx_handler_t m_usb_rx_handler;
#define rx_buffer_size 128
char m_rx_buffer[rx_buffer_size];        //fragmented usb parts of messages

#define g_usb_message_size 256
char g_usb_message[g_usb_message_size];

bool g_is_port_open = false;
bool g_is_write_buffer_ready = true;

void usb_tx_from_buffer();

NRF_RINGBUF_DEF(usb_tx_ring_buf, 1024);

void stream_to_message(const char* msg,uint8_t size)
{
    static uint8_t usb_msg_count=0;
    for(int i=0;i<size;i++)
    {
        char c = msg[i];
        if( (c == '\r') || (c == '\n') || (c == 0) )
        {
            if(usb_msg_count > 0)
            {
                m_usb_rx_handler(g_usb_message,usb_msg_count);
                usb_msg_count = 0;
            }
        }
        else
        {
            if(usb_msg_count < g_usb_message_size)
            {
                g_usb_message[usb_msg_count++] = c;
            }
            else
            {
                usb_msg_count = 0;
                blink_red(650,200); //signal error - discarded message
            }
        }
    }
}


/**
 * @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t (headphones)
 * */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            //This starts listening and notify an event immediatly when 'any' size of data is available
            app_usbd_cdc_acm_read_any(p_cdc_acm,m_rx_buffer,rx_buffer_size);
            g_is_port_open = true;
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            g_is_port_open = false;
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            g_is_write_buffer_ready = true;
            usb_tx_from_buffer();
            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            size_t size;
            ret_code_t ret;
            do
            {
                size = app_usbd_cdc_acm_rx_size(p_cdc_acm);     //get the number of bytes waiting in the buffer provided in the previous read call
                stream_to_message(m_rx_buffer,size);            //provide the data to the next layer
                ret = app_usbd_cdc_acm_read_any(p_cdc_acm,m_rx_buffer,rx_buffer_size);  //initiate the next listening
            } while (ret == NRF_SUCCESS);   //if ret is success, then another cycle of checking read data size using it and initiate listening
            break;
        }
        default:
            break;
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
            ////bsp_board_led_off(LED_USB_RESUME);
            break;
        case APP_USBD_EVT_DRV_RESUME:
            ////bsp_board_led_on(LED_USB_RESUME);
            break;
        case APP_USBD_EVT_STARTED:

            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            ////bsp_board_leds_off();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_INFO("USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            NRF_LOG_INFO("USB power removed");
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            NRF_LOG_INFO("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}

void usb_print_init(usb_rx_handler_t handler)
{
    ret_code_t ret;

    m_usb_rx_handler = handler;

    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };

    app_usbd_serial_num_generate();

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);
    NRF_LOG_INFO("USBD CDC ACM example started.");

    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);

    if (USBD_POWER_DETECTION)
    {
        ret = app_usbd_power_events_enable();
        APP_ERROR_CHECK(ret);
    }
    else
    {
        NRF_LOG_INFO("No USB power detection enabled\r\nStarting USB now");

        app_usbd_enable();
        app_usbd_start();
    }

     nrf_ringbuf_init(&usb_tx_ring_buf);
}

void usb_print_loop()
{
    while (app_usbd_event_queue_process())
    {
        /* Nothing to do */
    }
}

void usb_tx_from_buffer()
{
    if(g_is_write_buffer_ready)
    {
        g_is_write_buffer_ready = false;

        uint8_t packet[64];
        size_t length_read = 64;
        ret_code_t err_code = nrf_ringbuf_cpy_get(&usb_tx_ring_buf, packet, &length_read);

        ret_code_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm, packet, length_read);
        if( (err_code != NRF_SUCCESS) && (ret == NRF_ERROR_INVALID_STATE) )
        {
            blink_red(1000,200);
        }
    }
}

void usb_print(const void * p_buf,size_t length)
{
    if(!g_is_port_open)
    {
        blink_red(10,10);
        return;
    }

    size_t length_written = length;
    ret_code_t err_code = nrf_ringbuf_cpy_put(&usb_tx_ring_buf, p_buf, &length_written);
    if( (length_written == length) || (err_code == NRF_SUCCESS) )
    {
        usb_tx_from_buffer();
    }
    else
    {
        blink_red(100,10);
        return;
    }

}

char usb_print_buffer[256];

void usb_printf(const char *format, ...)
{
    va_list args;
    va_start (args, format);
    size_t length = vsnprintf(usb_print_buffer,256,format, args);
    va_end (args);

    usb_print(usb_print_buffer, length);
}