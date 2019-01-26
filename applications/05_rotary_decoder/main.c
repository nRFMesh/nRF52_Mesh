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

#include "nrf_drv_qdec.h"

#include "nrf_drv_gpiote.h"
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
#include "app_ser.h"
#include "timestamp.h"
#include "compare.h"
#include "utils.h"


void app_mesh_broadcast(message_t* msg);
void app_mesh_message(message_t* msg);

char rtc_message[64];
char rf_message[64];
char uart_message[64];
uint32_t uart_rx_size=0;

extern uint32_t ser_evt_tx_count;

static volatile bool m_report_ready_flag = false;
static volatile bool m_first_report_flag = true;
static volatile uint32_t m_accdblread;
static volatile int32_t m_accread;
static volatile int32_t m_pos;
static volatile int32_t g_pos_A = 1200;
static volatile int32_t g_pos_B = 1200;
static volatile int32_t g_pos_H = 1200;
static volatile uint32_t g_capture_time = 0;
static volatile bool m_capture = false;

static uint32_t g_time_now = 0;
static uint32_t g_time_next = 0;
static uint32_t g_time_log = 0;
static uint32_t g_loop_count = 0;
static bool is_expecting_sync = false;

//A - Green - 29 - scl
#define PIN_ENCODER_A 29
//B - White - 30 - sda
#define PIN_ENCODER_B 30
#define PIN_ENCODER_Debug 31

static nrf_ppi_channel_t ppi_chan_EncoderA_Timestamp;
static nrf_ppi_channel_t ppi_chan_RFrx_ClearTimestamp;
//redeclared here for ppi usage
const nrf_drv_timer_t TIMER_TIMESTAMP_APP = NRF_DRV_TIMER_INSTANCE(TIMESTAMP_TIMER_INSTANCE);

const nrf_drv_timer_t TIMER_COMPARE_APP = NRF_DRV_TIMER_INSTANCE(COMPARE_TIMER_INSTANCE);

//!!!!! BUG in SDK !!!!!
//uint32_t task1  = nrf_drv_timer_capture_task_address_get(&TIMER_TIMESTAMP_APP,NRF_TIMER_TASK_CAPTURE1);
//task1 had a value of 0x40008050 referring to capture4 in stead of following 0x40008044 as in datasheet
#define PPI_Task_Timer0_Capture1    0x40008044

#define PPI_Event_RADIO_END         0x4000110C
#define PPI_Task_Timer0_Clear       0x4000800C

void ppi_init()
{

    nrf_drv_ppi_init();

    //Encoder-A-pio => Timer0_Capture
    nrf_drv_ppi_channel_alloc(&ppi_chan_EncoderA_Timestamp);
    uint32_t event1 = nrf_drv_gpiote_in_event_addr_get(PIN_ENCODER_A);
    nrf_drv_ppi_channel_assign(ppi_chan_EncoderA_Timestamp, event1, PPI_Task_Timer0_Capture1);
    nrf_drv_ppi_channel_enable(ppi_chan_EncoderA_Timestamp);

    //RF-Rx packet => Timer0_Clear
    nrf_drv_ppi_channel_alloc(&ppi_chan_RFrx_ClearTimestamp);
    nrf_drv_ppi_channel_assign(ppi_chan_RFrx_ClearTimestamp, PPI_Event_RADIO_END, PPI_Task_Timer0_Clear);
    //usage : nrf_drv_ppi_channel_enable/disable(ppi_chan_RFrx_ClearTimestamp);
}

void rf_mesh_interrupt()
{
    if(is_expecting_sync)
    {
        g_loop_count = 0;
    }
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
        is_expecting_sync = true;
    }
    if(msg->pid == 0x41)//sync
    {
        if(is_expecting_sync)
        {
            nrf_drv_ppi_channel_disable(ppi_chan_RFrx_ClearTimestamp);
            is_expecting_sync = false;
            sprintf(rf_message,"sync:ok;ts:%lu",timestamp_get());
            mesh_bcast_text(rf_message);
        }
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

    if(strbegins(msg,"encoder:"))
    {
        //encoder_handle_raw(data,data_size);//TODO
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
}    


void app_mesh_message(message_t* msg)
{
}

void log_count(uint32_t count)
{
    sprintf(rf_message,"ts:%lu;loop:%lu",timestamp_get(),count);
    mesh_bcast_text(rf_message);
}

void log_steps()
{
    sprintf(rf_message,"tp:encoder;ts:%lu;pos:%ld",g_capture_time,g_pos_A%600);
    mesh_bcast_text(rf_message);
}

static void qdec_event_handler(nrf_drv_qdec_event_t event)
{
    /*if (event.type == NRF_QDEC_EVENT_REPORTRDY)
    {
        m_accdblread        = event.data.report.accdbl;
        m_accread           = event.data.report.acc;
        m_report_ready_flag = true;
    }
    if (event.type == NRF_QDEC_EVENT_SAMPLERDY)
    {
        m_pos+= event.data.sample.value;
        m_report_ready_flag = true;
    }*/
}

static nrf_drv_qdec_config_t const qdec_config =
{                                                                           
    .reportper          = NRF_QDEC_REPORTPER_10, 
    .sampleper          = (nrf_qdec_sampleper_t)NRFX_QDEC_CONFIG_SAMPLEPER, 
    .psela              = 29,                           
    .pselb              = 30,                           
    .pselled            = 31,
    .ledpre             = NRFX_QDEC_CONFIG_LEDPRE,                          
    .ledpol             = (nrf_qdec_ledpol_t)NRFX_QDEC_CONFIG_LEDPOL,       
    .interrupt_priority = NRFX_QDEC_CONFIG_IRQ_PRIORITY,                    
    .dbfen              = 1,
    .sample_inten       = 0                    
};

void init_decoder()
{
    uint32_t err_code;
    // Initialize hardware
    err_code = nrf_drv_qdec_init(&qdec_config, qdec_event_handler);
    NRF_QDEC->PSEL.LED = 0x8000001F;//Disconnect
    NRF_QDEC->SHORTS = 0;//No shortcuts
    APP_ERROR_CHECK(err_code);
    nrf_drv_qdec_enable();

}

void encoder_pin_A_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    //nrf_drv_gpiote_out_set(PIN_ENCODER_Debug);
    m_capture = true;
    g_capture_time = nrf_drv_timer_capture_get(&TIMER_TIMESTAMP_APP, 1);
    if(nrfx_gpiote_in_is_set(PIN_ENCODER_B))
    {
        g_pos_A++;
        g_pos_H++;
    }
    else
    {
        g_pos_A--;
        g_pos_H--;
    }
    //nrf_drv_gpiote_out_clear(PIN_ENCODER_Debug);
}

void encoder_pin_B_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    //nrf_drv_gpiote_out_set(PIN_ENCODER_Debug);
    m_capture = true;
    g_capture_time = nrf_drv_timer_capture_get(&TIMER_TIMESTAMP_APP, 2);
    if(nrfx_gpiote_in_is_set(PIN_ENCODER_A))
    {
        g_pos_B--;
        g_pos_H--;
    }
    else
    {
        g_pos_B++;
        g_pos_H++;
    }
    //nrf_drv_gpiote_out_clear(PIN_ENCODER_Debug);
}

void init_gpio_decoder()
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_gpio_cfg_output(PIN_ENCODER_Debug);

    nrf_drv_gpiote_in_config_t in_A_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_A_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_ENCODER_A, &in_A_config, encoder_pin_A_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_B_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_B_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_ENCODER_B, &in_B_config, encoder_pin_B_handler);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_gpiote_in_event_enable(PIN_ENCODER_A, true);
    //will not enable the B events, to leave enough cpu for A events
    //nrf_drv_gpiote_in_event_enable(PIN_ENCODER_B, true);
}

void apptimer_dummy_handler(nrf_timer_event_t event_type, void * p_context){}

void timestamp_compare1()
{
    nrf_gpio_pin_set(PIN_ENCODER_Debug);  
}

void timestamp_compare2()
{
    static uint32_t g_loop_count = 0;
    nrf_gpio_pin_clear(PIN_ENCODER_Debug);

    if((g_loop_count % 1000) == 0)
    {
        g_capture_time = timestamp_get();
        m_capture = true;
    }
    if(m_capture)
    {
        log_steps();
        m_capture = false;
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

    apptimer_config_t apptimer_cfg = {
        .cycle          = 4000,
        .offset1        = 1000,
        .call1          = (app_compare_handler_t)timestamp_compare1,
        .offset2        = 2000,
        .call2          = (app_compare_handler_t)timestamp_compare2
    };

    timestamp_init();
    compare_init(apptimer_cfg);

    //nrf_gpio_cfg_output(11); Debug pios 11,12,14,29

    ser_init(app_serial_handler);

    //Cannot use non-blocking with buffers from const code memory
    //reset is a status which single event is reset, that status takes the event name
    sprintf(rtc_message,"nodeid:%d;channel:%d;reset:1;ts:%lu\r\n",get_this_node_id(),mesh_channel(),timestamp_get());
    ser_send(rtc_message);

    err_code = mesh_init(rf_mesh_handler,mesh_cmd_response,rf_mesh_interrupt);
    APP_ERROR_CHECK(err_code);

    //only allow interrupts to start after init is done
    rtc_config(app_rtc_handler);

    mesh_tx_reset();
    mesh_ttl_set(0);

    nrf_delay_ms(200);

    //init_decoder();
    init_gpio_decoder();

    ppi_init();

    // ------------------------- Start Events ------------------------- 
    while(true)
    {
        mesh_consume_rx_messages();
    }
}
/*lint -restore */
