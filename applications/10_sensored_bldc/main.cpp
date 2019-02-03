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
#include <string>

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

#include "strmap.hpp"
#include "usb_print.hpp"
#include "bldc.hpp"




#define PWM_INSTANCE 0
#define GPIO_M_P1 NRF_GPIO_PIN_MAP(0,10)
#define GPIO_M_P2 NRF_GPIO_PIN_MAP(0,9)
#define GPIO_M_P3 NRF_GPIO_PIN_MAP(1,0)
#define GPIO_M_EN NRF_GPIO_PIN_MAP(0,24)

bldc_c motor(PWM_INSTANCE,GPIO_M_P1,GPIO_M_P2,GPIO_M_P3);

void app_usb_rx_handler(const char*msg,uint8_t size);

usb_c usb(app_usb_rx_handler);

void app_usb_rx_handler(const char*msg,uint8_t size)
{
    strmap_c params(msg,size);
    if(params.topic.compare("motor") == 0)
    {
        if(params.has("norm"))
        {
            float norm = std::stof(params["norm"]);
            motor.set_norm(norm);
        }
        if(params.has("target"))
        {
            float target = std::stof(params["target"]);
            motor.set_target(target);
        }
        if(params.has("speed"))
        {
            float speed = std::stof(params["speed"]);
            motor.set_speed(speed);
        }
        usb.printf("motor;norm:%0.2f;target:%0.2f;speed:%0.2f;\r\n",
                        motor.norm,
                        motor.absolute_target,
                        motor.rot_per_sec);
    }
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
    usb.printf("app:bldc;id:%lu;alive:%lu\r\n",get_this_node_id(),alive_count++);
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

    nrf_gpio_cfg_output(GPIO_M_EN);
    nrf_gpio_pin_set(GPIO_M_EN);

    // ------------------------- Start Init ------------------------- 
    usb.printf("bldc;reset:1\r\n");//will be lost if port is closed
    rtc_config(app_rtc_handler);


    // ------------------------- Start Events ------------------------- 
    uint32_t count = 0;
    while(true)
    {
        usb.loop();
        nrf_delay_us(1000);
        if((count % 500) == 0)
        {
            usb.printf("motor.absolute_steps:%0.3f\r\n",motor.absolute_steps);
        }
        count++;
    }
}
/*lint -restore */
