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
#include "mesh.h"

#include "nrf_mtx.h"

#include "usb_print.h"

}

char rtc_message[64];
char uart_message[64];
uint32_t uart_rx_size=0;

#define GPIO_CUSTOM_Debug 10


/**
 * @brief callback from the RF Mesh stack on valid packet received for this node
 * context: from main()->mesh_consume_rx_messages()->mesh_rx_handler()
 * 
 * @param msg message structure with packet parameters and payload
 */
void rf_mesh_handler(message_t* msg)
{
    led2_green_on();
    bool is_relevant_host = false;
    NRF_LOG_INFO("rf_mesh_handler()");
    if(MESH_IS_BROADCAST(msg->control))
    {
        is_relevant_host = true;
    }
    else if((msg->dest == get_this_node_id()))
    {
        if(MESH_IS_RESPONSE(msg->control))
        {
            is_relevant_host = true;
        }
        //else it's a request or a message
        else if( (msg->pid == Mesh_Pid_ExecuteCmd) && (UICR_is_rf_cmd()) )
        {
            mesh_execute_cmd(msg->payload,msg->payload_length,true,msg->source);
        }
    }
    if(is_relevant_host)
    {
        //Pure routers should not waste time sending messages over uart
        if(UICR_is_rf2uart())
        {
            char rf_message[128];
            mesh_parse(msg,rf_message);
            nrf_gpio_pin_set(GPIO_CUSTOM_Debug);
            usb_print(rf_message,strlen(rf_message));
            nrf_gpio_pin_clear(GPIO_CUSTOM_Debug);
        }
    }
    led2_green_off();
}

/**
 * @brief called only with a full line message coming from UART 
 * ending with '\r', '\n' or '0'
 * context: cdc_acm_user_ev_handler()
 * @param msg contains a pointer to the DMA buffer, so do not keep it after the call
 * @param size safe managemnt with known size, does not include the ending char '\r' or other
 */
//#define UART_MIRROR
void app_usb_rx_handler(const char*msg,uint8_t size)
{
    uart_rx_size+= size;

    if(UICR_is_uart_cmd())
    {
        mesh_text_request(msg,size);
    }
}

/**
 * @brief Callback from after completion of the cmd request
 * Note this is a call back as some undefined latency and events might happen
 * before the response is ready, thus the requests cannot always return immidiatly
 * context: main()-->rf_mesh_handler()->mesh_execute_cmd()
 *          cdc_acm_user_ev_handler()->app_usb_rx_handler()->mesh_text_request()
 * 
 * @param text 
 * @param length 
 */
void mesh_cmd_response(const char*text,uint8_t length)
{
    memcpy(uart_message,text,length);
    length+=sprintf(uart_message+length,"\r\n");//Add line ending and NULL terminate it with sprintf
    usb_print(uart_message,length);
}


/**
 * @brief application rtc event which is a configurable period delay
 * through the uicr config "sleep" in the nodes databse
 * 
 */
void app_rtc_handler()
{
    led2_green_on();
    uint32_t alive_count = mesh_tx_alive();//returns an incrementing counter
    usb_printf("id:%d:alive:%lu\r\n",get_this_node_id(),alive_count);
    led2_green_off();
}

int main(void)
{
    //--------------------- Important UICR settings --------------------- 
    //UICR.NFCPINS = 0xFFFFFFFE - Disabled
    //UICR.REGOUT0 = 0xFFFFFFFD - 3.3 V

    clocks_start();

    bsp_board_init(BSP_INIT_LEDS);
    nrf_gpio_cfg_output(GPIO_CUSTOM_Debug);

    blink_red(100,200);
    blink_green(100,200);
    blink_blue(100,200);


    // ------------------------- Start Init ------------------------- 

    usb_print_init(app_usb_rx_handler);

    mesh_init(rf_mesh_handler,mesh_cmd_response);

    rtc_config(app_rtc_handler);

    mesh_tx_reset();

    // ------------------------- Start Events ------------------------- 
    while(true)
    {
        //not optimal, should rather consume one rf message, then process one usb event
        mesh_consume_rx_messages();
        usb_print_loop();
    }
}
/*lint -restore */
