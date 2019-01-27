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

#include "nrf_drv_timer.h"
#include "nrf_drv_ppi.h"
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
#include "bldc.h"
#include "app_ser.h"
#include "timestamp.h"
#include "compare.h"
#include "utils.h"


void app_mesh_broadcast(message_t* msg);
void app_mesh_message(message_t* msg);

void rov_handle_raw(uint8_t *payload,uint8_t length);

char rtc_message[64];
char rf_message[64];
char uart_message[64];
uint32_t uart_rx_size=0;

extern uint32_t ser_evt_tx_count;

static bool     g_sync_perform = false;

#define GPIO_Sync_Controller 12

static nrf_ppi_channel_t ppi_chan_RFrx_ClearTimestamp;
static nrf_ppi_channel_t ppi_chan_Timer4Comp5_Clear;
//redeclared here for ppi usage
const nrf_drv_timer_t TIMER_TIMESTAMP_APP = NRF_DRV_TIMER_INSTANCE(TIMESTAMP_TIMER_INSTANCE);
const nrf_drv_timer_t TIMER_COMPARE_APP = NRF_DRV_TIMER_INSTANCE(COMPARE_TIMER_INSTANCE);

#define PPI_Event_RADIO_END         0x4000110C
#define PPI_Task_Timer0_Clear       0x4000800C

#define PPI_Event_Timer4_Compare5   0x4001B154
#define PPI_Task_Timer4_Clear       0x4001B00C


void ppi_init()
{

    nrf_drv_ppi_init();

    //RF-Rx packet => Timer0_Clear
    nrf_drv_ppi_channel_alloc(&ppi_chan_RFrx_ClearTimestamp);
    nrf_drv_ppi_channel_assign(ppi_chan_RFrx_ClearTimestamp, PPI_Event_RADIO_END, PPI_Task_Timer0_Clear);
    nrf_drv_ppi_channel_fork_assign(ppi_chan_RFrx_ClearTimestamp, PPI_Task_Timer4_Clear);
    //usage : nrf_drv_ppi_channel_enable/disable(ppi_chan_RFrx_ClearTimestamp);

    //Timer4_compare5 loops back to 0
    nrf_drv_ppi_channel_alloc(&ppi_chan_Timer4Comp5_Clear);
    nrf_drv_ppi_channel_assign(ppi_chan_Timer4Comp5_Clear, PPI_Event_Timer4_Compare5, PPI_Task_Timer4_Clear);
    nrf_drv_ppi_channel_enable(ppi_chan_Timer4Comp5_Clear);

}

/**
 * @brief callback from the RF Mesh stack on valid packet received for this node
 * 
 * @param msg message structure with packet parameters and payload
 */
void rf_mesh_handler(message_t* msg)
{
    if(msg->pid == 0x40)//sync-prepare
    {
        nrf_drv_ppi_channel_enable(ppi_chan_RFrx_ClearTimestamp);
    }
    if(msg->pid == 0x41)//sync
    {
        nrf_drv_ppi_channel_disable(ppi_chan_RFrx_ClearTimestamp);
        g_sync_perform = true;
    }

    bool is_relevant_host = false;
    NRF_LOG_INFO("rf_mesh_handler()");
    if(MESH_IS_BROADCAST(msg->control))
    {
        is_relevant_host = true;
        app_mesh_broadcast(msg);
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
        else
        {
            app_mesh_message(msg);
        }
    }
    if(is_relevant_host)
    {
        //Pure routers should not waste time sending messages over uart
        if(UICR_is_rf2uart())
        {
            char rf_message[128];
            mesh_parse(msg,rf_message);
            ser_send(rf_message);
        }
    }
}

/**
 * @brief called only with a full line message coming from UART 
 * ending with '\r', '\n' or '0'
 * @param msg contains a pointer to the DMA buffer, so do not keep it after the call
 * @param size safe managemnt with known size, does not include the ending char '\r' or other
 */
//#define UART_MIRROR
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

    if(strbegins(msg,"rov:"))
    {
        uint8_t data[32];//TODO define global max cmd size
        uint8_t data_size;
        if(text2bin(msg+4,size-4,data,&data_size))
        {
            rov_handle_raw(data,data_size);
        }
    }
    else if(UICR_is_uart_cmd())//here handle "msg:" and "cmd:"
    {
        mesh_text_request(msg,size);
    }
}

/**
 * @brief Callback from after completion of the cmd request
 * Note this is a call back as some undefined latency and events might happen
 * before the response is ready, thus the requests cannot always return immidiatly
 * 
 * @param text 
 * @param length 
 */
void mesh_cmd_response(const char*text,uint8_t length)
{
    memcpy(uart_message,text,length);
    sprintf(uart_message+length,"\r\n");//Add line ending and NULL terminate it with sprintf
    ser_send(uart_message);
}


/**
 * @brief application rtc event which is a configurable period delay
 * through the uicr config "sleep" in the nodes databse
 * 
 */
void app_rtc_handler()
{
    //corrupts other transmissions, unprotected critical sections
    //uint32_t alive_count = mesh_tx_alive();//returns an incrementing counter
    //NRF_LOG_INFO("id:%d:alive:%lu",get_this_node_id(),alive_count);
    #ifdef UART_SELF_ALIVE
        sprintf(rtc_message,"id:%d:alive:%lu;uart_rx:%lu\r\n",get_this_node_id(),alive_count,uart_rx_size);
        ser_send(rtc_message);
    #endif
    //UNUSED_VARIABLE(alive_count);
}

void app_mesh_broadcast(message_t* msg)
{
    if(msg->pid == 0x17)//bldc
    {
        sprintf(uart_message,"exec:app_mesh_broadcast()\r\n");//Add line ending and NULL terminate it with sprintf
        ser_send(uart_message);
    }
    else
    {
        //sprintf(uart_message,"exec:app_mesh_broadcast( unhandled )\r\n");//Add line ending and NULL terminate it with sprintf
        //ser_send(uart_message);
    }
}    


void app_mesh_message(message_t* msg)
{
    if(msg->pid == 0x17)//bldc
    {
        rov_handle_raw(msg->payload,msg->payload_length);
    }
}

void log_count(uint32_t count)
{
    sprintf(rf_message,"ts:%lu;loop:%lu",timestamp_get(),count);
    mesh_bcast_text(rf_message);
}
void log_bldc_pwm()
{
    uint16_t pwm1,pwm2,pwm3;
    bldc_pwm_get(&pwm1,&pwm2,&pwm3);
    sprintf(rf_message,"ts:%lu;tp:bldc;p1:%u",timestamp_get(),pwm1);
    mesh_bcast_text(rf_message);
}
//This function handles commands coming from either serial or rf
void rov_handle_raw(uint8_t *payload,uint8_t length)
{
    uint8_t alpha = *(payload);
    if(length == 2)
    {
        float norm = (float)payload[1] / 255.0;
        nrf_gpio_pin_set(GPIO_Sync_Controller);
        bldc_set_pole(alpha,norm);
        nrf_gpio_pin_clear(GPIO_Sync_Controller);
        //sprintf(uart_message,"len:%u;alpha:%u;norm:%0.2f\r\n",length,alpha,norm);
        //ser_send(uart_message);
        sprintf(rf_message,"ts:%lu;tp:bldc;alpha:%u",timestamp_get(),alpha);
        mesh_bcast_text(rf_message);
    }
}
void compare_call0()
{

}

void compare_call1()
{
    static uint32_t g_loop_count = 0;

    nrf_gpio_pin_clear(GPIO_Sync_Controller);
    if(g_sync_perform)
    {
        g_loop_count = 0;
        g_sync_perform = false;
    }
    if((g_loop_count % 1000) == 0)
    {
        log_bldc_pwm();
    }
    g_loop_count++;
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

    timestamp_init();
    bldc_init();

    nrf_gpio_cfg_output(GPIO_Sync_Controller);

    ser_init(app_serial_handler);

    //Cannot use non-blocking with buffers from const code memory
    //reset is a status which single event is reset, that status takes the event name
    sprintf(rtc_message,"nodeid:%d;channel:%d;reset:1;ts:%lu\r\n",get_this_node_id(),mesh_channel(),timestamp_get());
    ser_send(rtc_message);

    err_code = mesh_init(rf_mesh_handler,mesh_cmd_response);
    APP_ERROR_CHECK(err_code);

    //only allow interrupts to start after init is done
    rtc_config(app_rtc_handler);

    mesh_tx_reset();
    mesh_ttl_set(0);

    nrf_delay_ms(200);

    // ------------------------- Start Events ------------------------- 
    ppi_init();

    apptimer_config_t compare_cfg = {
        .offset0        = 1,//0 does not trigger due to 1 cycle PPI latency
        .call0          = (app_compare_handler_t)compare_call0,
        .offset1        = 1500,
        .call1          = (app_compare_handler_t)compare_call1,
        .cycle          = 4000
    };
    compare_init(compare_cfg);


    bldc_set_target(2*256*14);//half turn
    bldc_set_norm(0.5);//half turn
    bldc_set_speed(7.0 / 14);
    // ------------------------- Start Background loop ------------------------- 
    while(true)
    {
        mesh_consume_rx_messages();
    }
}
/*lint -restore */
