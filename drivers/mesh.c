/** @file mesh.c
 *
 * @author Wassim FILALI
 *
 * @compiler arm gcc
 *
 *
 * $Date: 31.05.2018 adding doxy header and mesh commander functions
 *
*/

#include "mesh.h"

#include "sdk_common.h"
#include "sdk_config.h"

#include "boards.h"

#include "nrf_esb.h"
#include "nrf_esb_error_codes.h"

#include "nrf_drv_timer.h"

#include "nrf_delay.h"
#include <stdio.h>
//for sprint_buf
#include "utils.h"

#if defined( __GNUC__ ) && (__LINT__ == 0)
    // This is required if one wants to use floating-point values in 'printf'
    // (by default this feature is not linked together with newlib-nano).
    // Please note, however, that this adds about 13 kB code footprint...
    __ASM(".global _printf_float");
#endif

#define NRF_LOG_MODULE_NAME mesh

#if (MESH_CONFIG_LOG_ENABLED == 1)
#define NRF_LOG_LEVEL MESH_CONFIG_LOG_LEVEL
#else //MESH_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif //MESH_CONFIG_LOG_ENABLED

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();



const char * const pid_name[] = {  "",          //0x00
                                "ping",         //0x01
                                "request_pid",  //0x02
                                "0x03",         //0x03
                                "reset",        //0x04
                                "alive",        //0x05
                                "button",       //0x06
                                "light",        //0x07
                                "temperature",  //0x08
                                "heat",         //0x09
                                "bme",          //0x0A
                                "rgb",          //0x0B
                                "magnet",       //0x0C
                                "dimmer",       //0x0D
                                "light_rgb",    //0x0E
                                "gesture",      //0x0F
                                "proximity",    //0x10
                                "humidity",     //0x11
                                "pressure",     //0x12
                                "acceleration", //0x13
                                "light_n",      //0x14
                                "Battery",      //0x15
                                "" };    

#include "uicr_user_defines.h"


static nrf_esb_config_t     nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
static nrf_esb_payload_t    tx_payload = NRF_ESB_CREATE_PAYLOAD(0, 0x01, 0x00);
static nrf_esb_payload_t    rx_payload;
static message_t rx_msg;
static volatile bool esb_completed = false;
static volatile bool esb_tx_complete = false;
static uint8_t g_ttl = 2;

static app_mesh_rf_handler_t m_app_rf_handler;

static app_mesh_cmd_handler_t m_app_cmd_handler;

//forward internal declarations
void mesh_tx_message(message_t* msg);
uint32_t mesh_tx_ack(message_t* msg, uint8_t ttl);
uint32_t mesh_forward_message(message_t* msg);
bool window_check_retransmit();
uint8_t cmd_parse_response(char* text,uint8_t*data,uint8_t size);

//-------------------------------------------------------------
//----------------------- Payload Store -----------------------
//-------------------------------------------------------------
#if (MESH_TIMER_ENABLED == 1)
const nrf_drv_timer_t TIMER_ACK = NRF_DRV_TIMER_INSTANCE(MESH_TIMER_INSTANCE);

/**
 * @brief Handler for timer events.
 */
void timer_ack_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            if(!window_check_retransmit())
            {
                nrf_drv_timer_disable(&TIMER_ACK);
            }
            break;
        default:
            //Do nothing.
            break;
    }
}
#endif /*MESH_TIMER_ENABLED*/



typedef struct
{
    bool is_waiting_ack;
    uint32_t timeout;
    uint32_t count;
    nrf_esb_payload_t payload;
}esb_payload_store_t;

#define PAYLOAD_STORE_SIZE 2

static esb_payload_store_t  tx_payload_window[PAYLOAD_STORE_SIZE];

/**
 * @brief Get the tx payload object from the Store in case a retransmission might be required
 * Otherwise the global tx_payload if no retransmission required or none available on which case
 * re-transmission would not be performed
 * 
 * @param control 
 * @return nrf_esb_payload_t* 
 */
nrf_esb_payload_t* window_get_payload(uint8_t control)
{
    nrf_esb_payload_t* res = NULL;
    if((UICR_RTX_Timeout != 0xFFFFFFFF) && (UICR_RTX_Count != 0xFFFFFFFF))
    {
        if((UICR_RTX_Timeout != 0) && (UICR_RTX_Count != 0))
        {
            if(MESH_WANT_ACKNOWLEDGE(control))
            {
                //find one free and assign it
                for(int i=0;(i<PAYLOAD_STORE_SIZE)&&(res==NULL);i++)
                {
                    if(!tx_payload_window[i].is_waiting_ack)
                    {
                        res = &tx_payload_window[i].payload;
                        tx_payload_window[i].is_waiting_ack = true;
                        tx_payload_window[i].timeout    = UICR_RTX_Timeout;
                        tx_payload_window[i].count      = UICR_RTX_Count;
                    }
                }
            }
        }
    }

    if(res == NULL)//in both fails from control test or no store available test
    {
        res = &tx_payload;
    }
    else
    {
        #if(MESH_TIMER_ENABLED == 1)
        nrf_drv_timer_enable(&TIMER_ACK);
        #endif /*MESH_TIMER_ENABLED*/
    }

    return res;
}

bool is_matching_msg_ack(message_t* msg,nrf_esb_payload_t *p_payload)
{
    if(msg->pid != p_payload->data[2])//0:Length, 1:Ctrl, 2:pid
    {
        return false;
    }
    if(msg->source != p_payload->data[4])//compare source with dest (dest is @ 4)
    {
        return false;
    }
    if(msg->dest != p_payload->data[3])//compare dest with source (source is @ 3)
    {
        return false;
    }
    return true;    
}

void window_remove_payload(message_t* msg)
{
    bool found = false;
    for(int i=0;(i<PAYLOAD_STORE_SIZE)&&(!found);i++)
    {
        if(tx_payload_window[i].is_waiting_ack)
        {
            if(is_matching_msg_ack(msg,&tx_payload_window[i].payload))
            {
                //TODO if serial cmd request, should report the rtx count
                tx_payload_window[i].is_waiting_ack = false;
                found = true;
            }
        }
    }
}

/**
 * @brief 
 * 
 * @return true the timer is still required
 * @return false no timeout still required
 */
bool window_check_retransmit()
{
    bool timer_still_required = false;
    for(int i=0;i<PAYLOAD_STORE_SIZE;i++)
    {
        if(tx_payload_window[i].is_waiting_ack)
        {
            bool this_timeout_is_still_required = true;
            if(--tx_payload_window[i].timeout == 0)
            {
                //retransmit
                nrf_esb_write_payload(&tx_payload_window[i].payload);
                if(--tx_payload_window[i].count == 0)
                {
                    //retransmit count over !!!!! free the slot
                    tx_payload_window[i].is_waiting_ack = false;
                    this_timeout_is_still_required = false;
                }
                //restart a new timeout for the next count
                tx_payload_window[i].timeout      = UICR_RTX_Timeout;
            }
            if(this_timeout_is_still_required)
            {
                timer_still_required = true;
            }
        }
    }
    return timer_still_required;
}

//-------------------------------------------------------------
//------------------------- Mesh Core -------------------------
//-------------------------------------------------------------

uint8_t mesh_channel()
{
    return UICR_RF_CHANNEL;
}

uint8_t mesh_get_channel()
{
    return NRF_RADIO->FREQUENCY;
}

bool mesh_set_crc(uint8_t crc)
{
    switch(crc)
    {
        case NRF_ESB_CRC_16BIT:
            NRF_RADIO->CRCINIT = 0xFFFFUL;      // Initial value
            NRF_RADIO->CRCPOLY = 0x11021UL;     // CRC poly: x^16+x^12^x^5+1
            break;
        
        case NRF_ESB_CRC_8BIT:
            NRF_RADIO->CRCINIT = 0xFFUL;        // Initial value
            NRF_RADIO->CRCPOLY = 0x107UL;       // CRC poly: x^8+x^2^x^1+1
            break;
        
        case NRF_ESB_CRC_OFF:
            break;
        
        default:
            return false;
    }
    NRF_RADIO->CRCCNF = crc;
    return true;
}

uint8_t mesh_get_crc()
{
    return (uint8_t)NRF_RADIO->CRCCNF;
}

void mesh_pre_tx()
{
    if(UICR_is_listening())
    {
        nrf_esb_stop_rx();
        NRF_LOG_DEBUG("switch to IDLE mode that aloows TX");
    }

}

void mesh_post_tx()
{
    if(UICR_is_listening())
    {
        nrf_esb_start_rx();
        NRF_LOG_DEBUG("switch to RX mode");
    }
}

void mesh_message_2_esb_payload(message_t *msg,nrf_esb_payload_t *p_tx_payload)
{
    //esb only parameters
    p_tx_payload->noack    = true;//Never request an ESB acknowledge
    p_tx_payload->pipe     = 0;//pipe is the selection of the address to use

    p_tx_payload->data[1] = msg->control;
    p_tx_payload->data[2] = msg->pid;
    p_tx_payload->data[3] = msg->source;

    uint8_t start_payload;
    if(MESH_IS_BROADCAST(msg->control))
    {
        start_payload = MESH_Broadcast_Header_Length;
    }
    else
    {
        p_tx_payload->data[4] = msg->dest;
        start_payload = MESH_P2P_Header_Length;
    }

    //this is the total ESB packet length
    p_tx_payload->length   = msg->payload_length + start_payload;
    p_tx_payload->data[0] = p_tx_payload->length;
    memcpy( p_tx_payload->data+start_payload,
            msg->payload,
            msg->payload_length);
}

void mesh_esb_2_message_payload(nrf_esb_payload_t *p_rx_payload,message_t *msg)
{
    msg->control = p_rx_payload->data[1];
    msg->pid = p_rx_payload->data[2];
    msg->source = p_rx_payload->data[3];

    msg->rssi = p_rx_payload->rssi;
    //dest is processed by esb rx function
    uint8_t payload_start;
    if(MESH_IS_BROADCAST(msg->control))
    {
        msg->dest = 255;
        payload_start = 4;
    }
    else
    {
        msg->dest = p_rx_payload->data[4];
        payload_start = 5;
    }
    msg->payload_length = p_rx_payload->length - payload_start;

    if(msg->payload_length > 0)
    {
        msg->payload = p_rx_payload->data + payload_start;
    }
}

void mesh_rx_handler(message_t* msg)
{
    //The app gets everything - and at top for time sync
    if(m_app_rf_handler != NULL)
    {
        m_app_rf_handler(msg);
    }

    bool is_to_be_forwarded = false;
    if(msg->dest == UICR_NODE_ID)//current node id match
    {
        if(MESH_WANT_ACKNOWLEDGE(msg->control))
        {
            mesh_tx_ack(msg,2);
        }
        else if(MESH_IS_ACKNOWLEDGE(msg->control))
        {
            window_remove_payload(msg);//rx_payload is a global shared variable uniquely used during reception
        }
    }
    //only re-route messaegs directed to other than the current node itself
    else if(UICR_is_router())
    {
        is_to_be_forwarded = true;
    }
    
    //as the message forward is destructive it is to be done at the last step
    if(is_to_be_forwarded)
    {
        mesh_forward_message(msg);
    }
}

void mesh_consume_rx_messages()
{
    static uint32_t count = 0;
    // Get the most recent element from the RX FIFO.
    while(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
    {
        mesh_esb_2_message_payload(&rx_payload,&rx_msg);
        NRF_LOG_INFO("ESB Rx %d- pipe: (%d) -> pid:%d ; length:%d",count++,rx_payload.pipe,rx_payload.pid,rx_payload.length);
        NRF_LOG_INFO("HSM - src: (%d) -> pid:0x%02X ; length:%d",rx_msg.source,rx_msg.pid, rx_msg.payload_length);
        mesh_rx_handler(&rx_msg);
    }
}

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
            NRF_LOG_DEBUG("ESB TX SUCCESS EVENT");
            mesh_post_tx();
            esb_tx_complete = true;
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            NRF_LOG_DEBUG("ESB TX FAILED EVENT");
            (void) nrf_esb_flush_tx();
            mesh_post_tx();
            esb_tx_complete = true;
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            NRF_LOG_DEBUG("________________ESB RX RECEIVED EVENT________________");
            //Do nothing, handled in while loop
            //but return immidiatly so that more rx events are filled in the rx fifo
            break;
        default:
            esb_completed = true;
            NRF_LOG_ERROR("ESB Unhandled Event (%d)",p_event->evt_id);
            break;
    }

    esb_completed = true;
}


uint32_t mesh_init(app_mesh_rf_handler_t rf_handler,app_mesh_cmd_handler_t cmd_handler)
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

    m_app_rf_handler = rf_handler;
    m_app_cmd_handler = cmd_handler;

    nrf_esb_config.retransmit_count         = 0;
    nrf_esb_config.selective_auto_ack       = true;//false is not supported : payload.noack  decides
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.payload_length           = 8;//Not used by DPL as then the MAX is configured
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    if(UICR_is_listening())
    {
        nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
    }
    else
    {
        nrf_esb_config.mode                     = NRF_ESB_MODE_PTX;
    }

    nrf_esb_config.crc                      = NRF_ESB_CRC_16BIT;

    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_rf_channel(UICR_RF_CHANNEL);
    VERIFY_SUCCESS(err_code);

#if defined(BOARD_NRF52_DONGLE)
    err_code = nrf_esb_set_tx_power(RADIO_TXPOWER_TXPOWER_Pos4dBm);
    VERIFY_SUCCESS(err_code);
#endif

    tx_payload.length  = 8;
    tx_payload.pipe    = 0;
    tx_payload.data[0] = 0x00;

    NRF_LOG_INFO("nodeId %d",UICR_NODE_ID);
    NRF_LOG_INFO("channel %d",UICR_RF_CHANNEL);


    #if (MESH_TIMER_ENABLED == 1)
        nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
        err_code = nrf_drv_timer_init(&TIMER_ACK, &timer_cfg, timer_ack_event_handler);
        VERIFY_SUCCESS(err_code);

        uint32_t time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_ACK, 1);//1 ms

        nrf_drv_timer_extended_compare(
            &TIMER_ACK, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

        nrf_drv_timer_enable(&TIMER_ACK);
    #endif/*MESH_TIMER_ENABLED*/

    if(UICR_is_listening())
    {
        err_code = nrf_esb_start_rx();
        VERIFY_SUCCESS(err_code);
        NRF_LOG_INFO("UICR listening");
    }
    else
    {
        NRF_LOG_INFO("UICR Not listening");
    }


    return NRF_SUCCESS;
}

/**
 * @brief can be used before going into sleep as RADIO does not operate in low power
 * This will return either after success or failure event
 */
void mesh_wait_tx()
{
    while(!esb_completed);
}

//--------------------------------------------------------------------------
//------------------------- Messages Serialisation -------------------------
//--------------------------------------------------------------------------


/**
 * @brief required TX raw as it prevents partial copies before using the same mesh_tx_message
 * Note that the length is not expected to be the first byte here as it will be added
 * 
 * @param p_data 
 * @param size 
 */
void mesh_tx_raw(uint8_t* p_data,uint8_t size)
{
    mesh_pre_tx();

    nrf_esb_payload_t * p_tx_payload = window_get_payload(p_data[0]);//Starts with control as first byte
    p_tx_payload->length   = size + 1;//as size itself is now added
    p_tx_payload->data[0] = p_tx_payload->length;
    memcpy( p_tx_payload->data+1,
            p_data,
            size);

    esb_completed = false;//reset the check
    nrf_esb_write_payload(p_tx_payload);
}
/**
 * @brief Transmits a message structure
 * 
 * @param msg : the message structure to be converted to an esb buffer and sent through nrf_esb_write_payload
 */
void mesh_tx_message(message_t* p_msg)
{
    mesh_pre_tx();

    nrf_esb_payload_t * p_tx_payload = window_get_payload(p_msg->control);
    mesh_message_2_esb_payload(p_msg,p_tx_payload);

    esb_completed = false;//reset the check
    NRF_LOG_DEBUG("________________TX esb payload length = %d________________",p_tx_payload->data[0]);
    //should not wait for esb_completed here as does not work from ISR context

    //NRF_ESB_TXMODE_AUTO is used no need to call nrf_esb_start_tx()
    //which could be used for a precise time transmission
    nrf_esb_write_payload(p_tx_payload);
    
    //nrf_esb_start_tx();//as the previous mode does not start it due to .mode
}

/**
 * @brief Transmits a button message
 * 
 * @param state button state press 0 or release 1
 * @return uint32_t NRF_SUCCESS
 */
uint32_t mesh_tx_button(uint8_t state)
{
    message_t msg;

    msg.control = 0x80 | 2;         // broadcast | ttl = 2
    msg.pid     = Mesh_Pid_Button;
    msg.source  = UICR_NODE_ID;
    msg.payload = &state;       //this will only be used from within the context of this function
    msg.payload_length = 1;

    mesh_tx_message(&msg);

    return NRF_SUCCESS;
}


uint32_t mesh_tx_ack(message_t* msg, uint8_t ttl)
{
    message_t ack;

    ack.control = 0x40 | ttl;         // ack | ttl = 2
    ack.pid     = msg->pid;
    ack.source  = msg->dest;        // == UICR_NODE_ID
    ack.dest    = msg->source;
    ack.payload_length = 0;

    mesh_tx_message(&ack);

    return NRF_SUCCESS;
}

void alive_add_rtx_info(message_t* msg)
{
    uint8_t add_index = msg->payload_length;
    msg->payload[add_index++] = msg->rssi;
    msg->payload[add_index++] = get_this_node_id();
    msg->payload[add_index++] = NRF_RADIO->TXPOWER;
    msg->payload_length+=3;
}

uint32_t mesh_forward_message(message_t* msg)
{
    uint8_t ttl = msg->control & 0x0F;
    if(ttl>0)
    {
        //Add TTL
        ttl--;
        msg->control &= 0xF0;//clear ttl
        msg->control |= ttl;//set new ttl
        //Update Alive
        if(msg->pid == Mesh_Pid_Alive)//rework to add the new (rssi,nid,tx)
        {
            alive_add_rtx_info(msg);
        }
        //send it
        mesh_tx_message(msg);
    }

    return NRF_SUCCESS;
}


/**
 * @brief Sends a simple message that has no payload, by providing only the pid
 * 
 * @param pid the application protocol id : e.g. reset,...
 * @return uint32_t 
 */
uint32_t mesh_tx_pid(uint8_t pid)
{
    message_t msg;

    msg.control = 0x80 | 2;         // broadcast | ttl = 2
    msg.pid     = pid;
    msg.source  = UICR_NODE_ID;
    msg.payload_length = 0;

    mesh_tx_message(&msg);

    return NRF_SUCCESS;
}

/**
 * @brief Sends a reset message, used usually when a device wakes up from reset
 * Combined in a database, it keeps history of when the device was first started
 * Of whether the device has reset du to any error
 * 
 */
void mesh_tx_reset()
{
    mesh_tx_pid(Mesh_Pid_Reset);
}

void mesh_ttl_set(uint8_t ttl)
{
    g_ttl = ttl;
}

void mesh_response_data(uint8_t pid,uint8_t dest,uint8_t * data,uint8_t size)
{
    message_t msg;

    msg.control = 0x00 | g_ttl;         // response | ttl = g_ttl
    msg.pid     = pid;
    msg.source  = UICR_NODE_ID;
    msg.dest    = dest;
    msg.payload = data;
    msg.payload_length = size;

    mesh_tx_message(&msg);
}

void mesh_bcast_data(uint8_t pid,uint8_t * data,uint8_t size)
{
    message_t msg;

    msg.control = 0x80 | g_ttl;         // broadcast | ttl = g_ttl
    msg.pid     = pid;
    msg.source  = UICR_NODE_ID;
    msg.payload = data;
    msg.payload_length = size;

    mesh_tx_message(&msg);
}

//limited to 255
void mesh_bcast_text(char *text)
{
    uint8_t size = strlen(text);
    if(size>MAX_MESH_MESSAGE_SIZE)//truncate in case of long message
    {
        text[MAX_MESH_MESSAGE_SIZE-1] = '>';
        size = MAX_MESH_MESSAGE_SIZE;
    }
    mesh_bcast_data(Mesh_Pid_Text,(uint8_t*)text,size);
}

/**
 * @brief Broadcast an alive packet with associated payload information
 * The payload contains a livecounter (uint32_t) and the RF transmission power (int8_t)
 * 
 */
uint32_t mesh_tx_alive()
{
    static uint32_t live_count = 0;

    uint8_t data[5];
    data[0] = 0xFF & (uint8_t)(live_count >> 24);
    data[1] = 0xFF & (uint8_t)(live_count >> 16);
    data[2] = 0xFF & (uint8_t)(live_count >> 8);
    data[3] = 0xFF & (uint8_t)(live_count );
    data[4] = NRF_RADIO->TXPOWER;

    mesh_bcast_data(Mesh_Pid_Alive,data,5);
    
    return live_count++;//returns the first used value before the increment
}



void mesh_tx_light(uint32_t light)
{
    mesh_bcast_data(Mesh_Pid_Light,(uint8_t*)&light,4);
}

void mesh_tx_battery(uint16_t voltage)
{
    uint8_t data[2];
    data[0] = 0xFF & (uint8_t)(voltage >> 8);
    data[1] = 0xFF & (uint8_t)(voltage);
    mesh_bcast_data(Mesh_Pid_Battery,data,2);
}

void mesh_tx_bme(int32_t temp,uint32_t hum,uint32_t press)
{
    uint8_t data[12];
    data[0] = 0xFF & (uint8_t)(temp >> 24);
    data[1] = 0xFF & (uint8_t)(temp >> 16);
    data[2] = 0xFF & (uint8_t)(temp >> 8);
    data[3] = 0xFF & (uint8_t)(temp );
    data[4] = 0xFF & (uint8_t)(hum >> 24);
    data[5] = 0xFF & (uint8_t)(hum >> 16);
    data[6] = 0xFF & (uint8_t)(hum >> 8);
    data[7] = 0xFF & (uint8_t)(hum );
    data[8] = 0xFF & (uint8_t)(press >> 24);
    data[9] = 0xFF & (uint8_t)(press >> 16);
    data[10]= 0xFF & (uint8_t)(press >> 8);
    data[11]= 0xFF & (uint8_t)(press );
    mesh_bcast_data(Mesh_Pid_bme,data,12);
}

//----------------------------------------------------------------------------
//------------------------- Messages Deserialisation -------------------------
//----------------------------------------------------------------------------

/**
 * @brief parses a binary alive message that contains optionally a complete
 * trace route information of every jump it went through in the mesh
 * The first transmission give the info of the Tx_power, and each receiver complete
 * the triple info with rss and node id, then sends the triple (tx_power,rssi,nodeid)
 * to the next node
 * @param p_msg pointer where to generate the text message
 * @param data pointer to the payload of the alive message to parse
 * @param size size of the data
 * @param rssi detection from the current node, required as specific to the captured packet
 * @return int the number of characters printed in the first text pointer
 */
int rx_alive(char * p_msg,uint8_t*data,uint8_t size,int8_t rssi)
{
    if(size == 0)
    {
        return sprintf(p_msg,"alive:1");
    }
    else if((size >= 5) && ((size - 5) % 3 == 0))//size pattern of alive messages
    {
        //-------------------------- 3 sections --------------------------
        uint32_t live_count;
        int data_index = 0;
        live_count  = data[data_index++] << 24;
        live_count |= data[data_index++] << 16;
        live_count |= data[data_index++] << 8;
        live_count |= data[data_index++];
        //-------------------------- section one permanent --------------------------
        int nb_rtx = (size-5) / 3;//must be rounded here as % test passed
        int add = sprintf(p_msg,"alive:%lu;nb:%u",live_count,nb_rtx+1);
        p_msg += add;
        int total_print = add;
        //----------------------- section two depends on nb rtx -----------------------
        for(int i=0;i<nb_rtx;i++)
        {
            add = sprintf(p_msg,";rx%d:%d,-%d,%d",
                                                i+1,
                                                data[data_index],
                                                data[data_index+1],
                                                data[data_index+2]
                                                );
            data_index+=3;//to avoid the error of operation may be undefined with muliple increments in sprintf
            p_msg += add;
            total_print += add;
        }
        //----------------------- section three permanent, the last rtx is special -----------------------
        add = sprintf(p_msg,";rx%d:%d,-%d,%d",
                                            nb_rtx+1,
                                            data[data_index++],
                                            rssi,
                                            get_this_node_id()
                                            );
        total_print += add;
        return total_print;
    }
    else
    {
        return sprintf(p_msg,"size not 0, not 5 but:%d",size);
    }
}

int rx_button(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 1)
    {
        return sprintf(p_msg,"size not 1 but:%u",size);
    }
    else
    {
        return sprintf(p_msg,"button:%u",data[0]);
    }
}

/**
 * @brief Converts the light from 4 Big endien uint32_t mili-lux to printed float
 * 
 * @param p_msg output of the printed param:val
 * @param data pointer on the payload of the light parameter bytes
 * @param size size of the payload to assert the protocol coherency
 * @return int length of the printed string
 */
int rx_light(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size == 2)
    {
        uint16_t    light  = data[1] << 8;
                    light |= data[0];
        return sprintf(p_msg,"deprecated_light:%u",light);
    }
    else if(size == 4)
    {
        uint32_t    light  = data[3] << 24;
                    light |= data[2] << 16;
                    light |= data[1] <<  8;
                    light |= data[0];
        float f_light = light;
        f_light /= 1000;
        return sprintf(p_msg,"light:%.03f",f_light);
    }
    else
    {
        return sprintf(p_msg,"size not 4, not 2 but:%d",size);
    }
}

int rx_new_light(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size == 2)
    {
        uint16_t    light  = data[1] << 8;
                    light |= data[0];
        return sprintf(p_msg,"deprecated_new_light:%u",light);
    }
    else
    {
        return sprintf(p_msg,"size not 2 but:%d",size);
    }
}

int rx_led_rgb(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size == 3)
    {
        uint8_t    red  =      data[0];
        uint8_t    green  =    data[1];
        uint8_t    blue  =     data[2];
        return sprintf(p_msg,"led_r:%u;led_g:%u;led_b:%u",red,green,blue);
    }
    else if(size == 0)
    {
        return 0;//no addition as this is an ack
    }
    else
    {
        return sprintf(p_msg,"size not 8, not 0 but:%u",size);
    }
}

int rx_light_rgb(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 8)
    {
        return sprintf(p_msg,"size not 8 but:%u",size);
    }
    else
    {
        uint16_t    ambient  =  data[0] << 8;
                    ambient |=  data[1];
        uint16_t    red  =      data[2] << 8;
                    red |=      data[3];
        uint16_t    green  =    data[4] << 8;
                    green |=    data[5];
        uint16_t    blue  =     data[6] << 8;
                    blue |=     data[7];
        return sprintf(p_msg,"ambient:%u;red:%u;green:%u;blue:%u",ambient,red,green,blue);
    }
}

int rx_temperature(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 4)
    {
        return sprintf(p_msg,"size not 4 but:%d",size);
    }
    else
    {
        uint32_t    temperature  = data[3] << 24;
                    temperature |= data[2] << 16;
                    temperature |= data[1] <<  8;
                    temperature |= data[0];
        return sprintf(p_msg,"deprecated_temp:%lu",temperature);
    }
}

int rx_humidity(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 4)
    {
        return sprintf(p_msg,"size not 4 but:%d",size);
    }
    else
    {
        uint32_t    humidity  = data[3] << 24;
                    humidity |= data[2] << 16;
                    humidity |= data[1] <<  8;
                    humidity |= data[0];
        return sprintf(p_msg,"deprecated_hum:%lu",humidity);
    }
}

int rx_pressure(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 4)
    {
        return sprintf(p_msg,"size not 4 but:%d",size);
    }
    else
    {
        uint32_t    pressure  = data[3] << 24;
                    pressure |= data[2] << 16;
                    pressure |= data[1] <<  8;
                    pressure |= data[0];
        return sprintf(p_msg,"deprecated_press:%lu",pressure);
    }
}

int rx_bme(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 12)
    {
        if(size == 8)
        {
            int add = sprintf(p_msg,"deprecated_bme_reg:");
            p_msg += add;
            add+=sprint_buf(p_msg,(const char*)data,size);
            return add;
        }
        else
        {
            return sprintf(p_msg,"size not 12 but:%u",size);
        }
    }
    else
    {
        int32_t temp  = data[0] << 24;
                temp |= data[1] << 16;
                temp |= data[2] <<  8;
                temp |= data[3];
        int32_t mst = temp / 100;
        int32_t lst = temp % 100;
        if(lst<0)
        {
            lst*=-1;
        }
        uint32_t hum  = data[4] << 24;
                hum |= data[5] << 16;
                hum |= data[6] <<  8;
                hum |= data[7];
        uint32_t msh = hum>>10;
        uint32_t lsh = hum & 0x3FF;
        uint32_t press  = data[8]  << 24;
                press |= data[9]  << 16;
                press |= data[10] <<  8;
                press |= data[11];
        uint32_t msp = press / (256 * 100);
        uint32_t lsp = (press/256) % 100;
        return sprintf(p_msg,"temp:%ld.%02ld;hum:%lu.%03lu;press:%lu.%02lu",mst,lst,msh,lsh,msp,lsp);
    }
}

int rx_battery(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 2)
    {
        return sprintf(p_msg,"size not 2 but:%d",size);
    }
    else
    {
        uint16_t 	bat_val = data[0] << 8;
                    bat_val |= data[1];
        uint16_t msv = bat_val / 1000;
        uint16_t lsv = bat_val % 1000;
        return sprintf(p_msg,"battery:%u.%03u",msv,lsv);
    }
}

int rx_accell(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 6)
    {
        return sprintf(p_msg,"size not 6 but:%d",size);
    }
    else
    {
        int16_t 	accell_x =  data[0] << 8;
                    accell_x |= data[1];
        int16_t 	accell_y =  data[2] << 8;
                    accell_y |= data[3];
        int16_t 	accell_z =  data[4] << 8;
                    accell_z |= data[5];
        float   x =  accell_x;
                x /= 16384;
        float   y =  accell_y;
                y /= 16384;
        float   z =  accell_z;
                z /= 16384;
        //note only +-2g AFS is used
        //although one lsb is 0.00006 it is truncated to 0.001
        int sres = sprintf(p_msg,"accx:%0.3f;accy:%0.3f;accz:%0.3f",x,y,z);
        return sres;
    }
}

int rx_gyro(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 6)
    {
        return sprintf(p_msg,"size not 6 but:%d",size);
    }
    else
    {
        int16_t 	gyro_x =  data[0] << 8;
                    gyro_x |= data[1];
        int16_t 	gyro_y =  data[2] << 8;
                    gyro_y |= data[3];
        int16_t 	gyro_z =  data[4] << 8;
                    gyro_z |= data[5];
        float   x =  gyro_x;
                x /= 131;
        float   y =  gyro_y;
                y /= 131;
        float   z =  gyro_z;
                z /= 131;
        //note only +-250 °/s is used
        int sres = sprintf(p_msg,"gyrx:%0.3f;gyry:%0.3f;gyrz:%0.3f",x,y,z);
        return sres;
    }
}

int rx_text(char * p_msg,uint8_t*data,uint8_t size)
{
    memcpy(p_msg,data,size);
    return size;
}

void mesh_parse(message_t* msg,char * p_msg)
{ 
    p_msg += sprintf(  p_msg,"pid:%d;ctrl:0x%02X;src:%d;",msg->pid,msg->control,msg->source);
    if(!(MESH_IS_BROADCAST(msg->control)))
    {
        p_msg += sprintf(  p_msg,"dest:%d;",msg->dest);
    }
    if(MESH_IS_ACKNOWLEDGE(msg->control))
    {
        p_msg += sprintf(  p_msg,"ack:1;");
    }
    switch(msg->pid)
    {
        case Mesh_Pid_Alive:
            {
                p_msg += rx_alive(p_msg,msg->payload,msg->payload_length,msg->rssi);
            }
            break;
        case Mesh_Pid_Reset:
            {
                p_msg += sprintf(p_msg,"reset:1");
            }
            break;
        case Mesh_Pid_Button:
            {
                p_msg += rx_button(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_Light:
            {
                p_msg += rx_light(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_light_rgb:
            {
                p_msg += rx_light_rgb(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_led_rgb:
            {
                p_msg += rx_led_rgb(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_Temperature:
        {
                p_msg += rx_temperature(p_msg,msg->payload,msg->payload_length);
        }
        break;
        case Mesh_Pid_Humidity:
        {
                p_msg += rx_humidity(p_msg,msg->payload,msg->payload_length);
        }
        break;
        case Mesh_Pid_Pressure:
        {
                p_msg += rx_pressure(p_msg,msg->payload,msg->payload_length);
        }
        break;
        case Mesh_Pid_bme:
            {
                p_msg += rx_bme(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_accell:
            {
                p_msg += rx_accell(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_Battery:
            {
                p_msg += rx_battery(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_new_light:
            {
                p_msg += rx_new_light(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_Text:
            {
                p_msg += rx_text(p_msg,msg->payload,msg->payload_length);
            }
            break;
        case Mesh_Pid_ExecuteCmd:
            {
                //only responses directed to us are parsed
                p_msg += cmd_parse_response(p_msg,msg->payload,msg->payload_length);
            }
            break;
        default:
        {
            if(msg->payload_length > 0)
            {
                p_msg += sprintf(p_msg,"payload:");
                p_msg += sprint_buf(p_msg,(const char*)msg->payload,msg->payload_length);
            }
        }
        break;
    }
    sprintf(p_msg,"\r\n");
}

void mesh_parse_raw(message_t* msg,char * p_msg)
{ 
    int add;
    add = sprintf(p_msg,"rssi:-%d;nodeid:%d;control:0x%02X",msg->rssi,msg->source,msg->control);
    p_msg += add;
    if(msg->pid <= 0x15)//sizeof(pid_name)
    {
        add = sprintf(p_msg,";pid:%s",pid_name[msg->pid]);
        p_msg += add;
    }
    else
    {
        add = sprintf(p_msg,";pid:0x%02X",msg->pid);
        p_msg += add;
    }
    if(msg->payload_length > 0)
    {
        add = sprintf(p_msg,";length:%d;data:0x",msg->payload_length);
        p_msg += add;
    }
    for(int i=0;i<msg->payload_length;i++)
    {
        add = sprintf(p_msg,"%02X ",msg->payload[i]);
        p_msg += add;
    }
    sprintf(p_msg,"\r\n");
}

void mesh_parse_bytes(message_t* msg,char * p_msg)
{ 
    int add;
    add = sprintf(p_msg,"0x%02X 0x%02X 0x%02X 0x%02X",msg->control,msg->pid,msg->source,msg->dest);
    p_msg += add;
    //CRC takes two bytes
    if(msg->payload_length > 0)
    {
        add = sprintf(p_msg,"; (%d) payload: 0x",msg->payload_length);
        p_msg += add;
    }
    for(int i=0;i<msg->payload_length;i++)
    {
        add = sprintf(p_msg,"%02X ",msg->payload[i]);
        p_msg += add;
    }
    sprintf(p_msg,"\r\n");
}



//------------------------------------------------------------------
//------------------------- Mesh Commander -------------------------
//------------------------------------------------------------------

uint8_t cmd_parse_response(char* text,uint8_t*data,uint8_t size)
{
    uint8_t length;
    switch(data[0])
    {
        case MESH_cmd_node_id_set:
        {
            //TODO persistance of parameters
            length = sprintf(text,"cmd:set_node_id;set:%u;get:%u",data[1],data[2]);
        }
        break;
        case MESH_cmd_node_id_get:
        {
            length = sprintf(text,"cmd:get_node_id;node_id:%u",data[1]);
        }
        break;
        case MESH_cmd_rf_chan_set:
        {
            length = sprintf(text,"cmd:set_channel;set:%u;get:%u",data[1],data[2]);
        }
        break;
        case MESH_cmd_rf_chan_get:
        {
            length = sprintf(text,"cmd:get_channel;channel:%u",data[1]);
        }
        break;
        case MESH_cmd_crc_set:
        {
            length = sprintf(text,"cmd:set_crc;set:%u;get:%u",data[1],data[2]);
        }
        break;
        case MESH_cmd_crc_get:
        {
            length = sprintf(text,"cmd:get_crc;crc:%u",data[1]);
        }
        break;
        default:
        {
            length = sprintf(text,"cmd:0x%02X;resp:unknown",data[0]);
        }
        break;
    }
    return length;
}

/**
 * @brief Executes a command received in a binary format
 * 
 * @param data the array starting with <cmd_id> followed by <param0><param1>,...
 * @param size the total size including the first byte of cmd_id
 */
void mesh_execute_cmd(uint8_t*data,uint8_t size,bool is_rf_request,uint8_t rf_nodeid)
{
    uint8_t resp[32];
    uint8_t resp_len;
    resp[0] = data[0];          //First byte response is always the command id requested
    switch(data[0])
    {
        case MESH_cmd_node_id_set:
        {
            //TODO persistance of parameters
            resp[1] = data[1];          //set request confirmation
            resp[2] = get_this_node_id();   //new set value as read from source after set
            resp_len = 3;
        }
        break;
        case MESH_cmd_node_id_get:
        {
            resp[1] = get_this_node_id();   //Value as read from source after set
            resp_len = 2;
        }
        break;
        case MESH_cmd_rf_chan_set:
        {
            mesh_wait_tx();//in case any action was ongoing
            nrf_esb_stop_rx();
            nrf_esb_set_rf_channel(data[1]);
            nrf_esb_start_rx();
            resp[1] = data[1];          //set channel request confirmation
            resp[2] = mesh_get_channel();   //new set value as read from source after set
            resp_len = 3;
        }
        break;
        case MESH_cmd_rf_chan_get:
        {
            resp[1] = mesh_get_channel();   //new set value as read from source after set
            resp_len = 2;
        }
        break;
        case MESH_cmd_crc_set:
        {
            mesh_wait_tx();//in case any action was ongoing
            nrf_esb_stop_rx();
            mesh_set_crc(data[1]);
            nrf_esb_start_rx();
            resp[1] = data[1];              //set crc request confirmation
            resp[2] = mesh_get_crc();       //new set value as read from source after set
            resp_len = 3;
        }
        break;
        case MESH_cmd_crc_get:
        {
            resp[1] = mesh_get_crc();       //new set value as read from source after set
            resp_len = 2;
        }
        break;
        default:
        {
            resp_len = 1;
        }
        break;
    }
    if(is_rf_request)
    {
        mesh_response_data(Mesh_Pid_ExecuteCmd,rf_nodeid,resp,resp_len);
    }
    else
    {
        char text[128];
        uint8_t length = cmd_parse_response(text,resp,resp_len);
        m_app_cmd_handler(text,length);
    }
}

/**
 * @brief executes teh command immidiatly
 * Future extension should use a command fifo
 * 
 * @param text contains a command line to execute a mesh rf function
 * supported commands :
 * * msg:0x00112233445566... note length not included as will be generated
 * where 0:control , 1:source , 2:dest/payload , 3:payload,...
 * * cmd:0x00112233445566
 * where 0x00 is the command id
 * the rest are the command parameters including : crc_cfg, header_cfg, bitrate,...
 * @param length number of characters in msg
 */
void mesh_text_request(const char*text,uint8_t length)
{
    if(strbegins(text,"msg:"))
    {
        uint8_t data[32];//TODO define global max cmd size
        uint8_t size;
        if(text2bin(text+4,length-4,data,&size))
        {
            mesh_tx_raw(data,size);
            char resp[32];
            uint8_t resp_len = sprintf(resp,"sent_msg_len:%d",size);
            m_app_cmd_handler(resp,resp_len);
        }
    }
    else if(strbegins(text,"cmd:"))
    {
        uint8_t data[32];//TODO define global max cmd size
        uint8_t size;
        if(text2bin(text+4,length-4,data,&size))
        {
            mesh_execute_cmd(data,size,false,0);//rf_nodeid unused in this case, set as 0
        }
    }
}
