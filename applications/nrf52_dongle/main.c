/**
 * Home Smart Mesh
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_util.h"

//for the log
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// --------------------- inputs from sdk_config --------------------- 
// ---> TWI0_ENABLED ---> TWI1_ENABLED
#include "uicr_user_defines.h"
//drivers
//apps
#include "clocks.h"
#include "mesh.h"
#include "app_ser.h"

char rtc_message[100];
char uart_message[100];
char rf_message[100];
uint32_t uart_rx_size=0;

void blink()
{
    bsp_board_led_on(0);
    nrf_delay_ms(200);
    bsp_board_leds_off();
    nrf_delay_ms(200);
    bsp_board_led_on(0);
    nrf_delay_ms(200);
    bsp_board_leds_off();
}

void rf_mesh_handler(message_t* msg)
{
    NRF_LOG_INFO("rf_mesh_handler()");

    mesh_parse(msg,rf_message);
    ser_send(rf_message);
}

/**
 * @brief called only with a full line message ending with '\r', '\n' or '0'
 * 
 * @param msg contains a pointer to the DMA buffer, so do not keep it after the call
 * @param size safe managemnt with known size
 */
#define UART_MIRROR
void app_serial_handler(const char*msg,uint8_t size)
{
    uart_rx_size+= size;
    //the input (msg) is really the RX DMA pointer location
    //and the output (uart_message) is reall the TX DMA pointer location
    //so have to copy here to avoid overwriting
    #ifdef UART_MIRROR
        memcpy(uart_message,msg,size);
        sprintf(uart_message+size,"\r\n");//Add line ending and NULL terminate it with sprintf
        ser_send(uart_message);
    #endif

    if(UICR_is_rf_cmd())
    {
        mesh_handle_cmd(msg,size);
    }
}

void app_rtc_handler()
{
    uint32_t alive_count = mesh_tx_alive();//returns an incrementing counter
    NRF_LOG_INFO("id:%d:alive:%lu",mesh_node_id(),alive_count);

    sprintf(rtc_message,"id:%d:alive:%lu;uart_rx:%lu\r\n",mesh_node_id(),alive_count,uart_rx_size);
    ser_send(rtc_message);
}

int main(void)
{
    uint32_t err_code;

    // ------------------------- Start Init ------------------------- 
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("__________________________________");
    NRF_LOG_INFO("Hello from the nRF52 UART Dongle");
    NRF_LOG_INFO("__________________________________");


    clocks_start();
    bsp_board_init(BSP_INIT_LEDS);
    ser_init(app_serial_handler);

    //Cannot use non-blocking with buffers from const code memory
    sprintf(rtc_message,"____________________________________\r\n");
    ser_send(rtc_message);
    sprintf(rtc_message,"nodeid:%d;channel:%d;event:reset\r\n",mesh_node_id(),mesh_channel());
    ser_send(rtc_message);

    blink();

    err_code = mesh_init(rf_mesh_handler);
    APP_ERROR_CHECK(err_code);

    //only allow interrupts to start after init is done
    rtc_config(app_rtc_handler);

    mesh_tx_reset();

    // ------------------------- Start Events ------------------------- 

    while(true)
    {
        __WFE();
    }
}
/*lint -restore */
