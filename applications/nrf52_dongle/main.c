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

//drivers
//apps
#include "clocks.h"
#include "mesh.h"
#include "app_ser.h"

char message[50];

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

void app_mesh_handler(message_t* msg)
{
    NRF_LOG_INFO("app_mesh_handler()");

    sprintf(message,"rssi:-%d;nodeid:%d;pid:0x%02X\r\n",msg->rssi,msg->source,msg->pid);
    ser_send(message);
}

void app_rtc_handler()
{
    NRF_LOG_INFO("RTC Tick");
    mesh_tx_alive();
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
    ser_init();

    ser_send("____________________________________\r\n.\r\n");
    nrf_delay_ms(10);
    sprintf(message,"nodeid:%d;channel:%d;event:reset\r\n.\r\n",mesh_node_id(),mesh_channel());
    ser_send(message);

    blink();

    err_code = mesh_init(app_mesh_handler);
    APP_ERROR_CHECK(err_code);

    //only allow interrupts to start after init is done
    rtc_config(app_rtc_handler);

    mesh_tx_reset();

    // ------------------------- Start Events ------------------------- 

    while(true)
    {
        nrf_delay_ms(100);
    }
}
/*lint -restore */
