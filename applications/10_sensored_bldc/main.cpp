/** @file main.c
 *
 * main entry for the application nRF52_dongle
 * 
 * @author Wassim FILALI
 *
 * @compiler arm gcc
 *
 *
 * $Date: 01.06.2018 adding doxy header this file existed since the repo creation
 *
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
extern "C"
{
#include "sdk_config.h"

//#include "nrf.h"
//#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_util.h"

#include "bsp.h"


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
#include "utils.h"
}

#include "usb_print.hpp"

char rtc_message[64];
uint32_t uart_rx_size=0;


void app_usb_rx_handler(const char*msg,uint8_t size);

usb_c usb(app_usb_rx_handler);

void app_usb_rx_handler(const char*msg,uint8_t size)
{

}


/**
 * @brief application rtc event which is a configurable period delay
 * through the uicr config "sleep" in the nodes databse
 * 
 */
void app_rtc_handler()
{
    static uint32_t alive_count = 0;
    led2_green_on();
    usb.printf("bldc;alive:%lu\r\n",alive_count++);
    led2_green_off();
}

int main(void)
{
    //--------------------- Important UICR settings --------------------- 
    //UICR.NFCPINS = 0xFFFFFFFE - Disabled
    //UICR.REGOUT0 = 0xFFFFFFFD - 3.3 V

    clocks_start();

    bsp_board_init(BSP_INIT_LEDS);

    blink_red(1000,200);
    blink_green(1000,200);
    blink_blue(1000,200);


    // ------------------------- Start Init ------------------------- 
    usb.printf("bldc;reset:1\r\n");
    rtc_config(app_rtc_handler);

    // ------------------------- Start Events ------------------------- 
    while(true)
    {
        usb.loop();
    }
}
/*lint -restore */
