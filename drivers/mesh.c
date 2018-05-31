
#include "mesh.h"

#include "sdk_common.h"

#include "boards.h"

#include "nrf_esb.h"
#include "nrf_esb_error_codes.h"

#include "nrf_delay.h"
#include <stdio.h>


#define NRF_LOG_MODULE_NAME mesh

#if (MESH_CONFIG_LOG_ENABLED == 1)
#define NRF_LOG_LEVEL MESH_CONFIG_LOG_LEVEL
#else //MESH_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif //MESH_CONFIG_LOG_ENABLED

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();


#define Mesh_Pid_Alive 0x05
#define Mesh_Pid_Reset 0x04
#define Mesh_Pid_Button 0x06

//light was 16bit redefined u32 bit
#define Mesh_Pid_Light 0x07
//bme280 no more with uncalibrated params
#define Mesh_Pid_bme        0x0A
#define Mesh_Pid_Battery    0x15

#define MESH_IS_BROADCAST(val) ((val & 0x80) == 0x80)

#define MESH_Broadcast_Header_Length 4
#define MESH_P2P_Header_Length 5


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

#define UICR_NODE_ID    NRF_UICR->CUSTOMER[0]
#define UICR_RF_CHANNEL NRF_UICR->CUSTOMER[1]
#define UICR_LISTENING  NRF_UICR->CUSTOMER[3]
#define UICR_is_router()  (NRF_UICR->CUSTOMER[4] == 0xBABA)

static nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
static nrf_esb_payload_t tx_payload = NRF_ESB_CREATE_PAYLOAD(0, 0x01, 0x00);
static nrf_esb_payload_t rx_payload;
static message_t rx_msg;
static volatile bool esb_completed = false;
static volatile bool esb_tx_complete = false;

static app_mesh_handler_t m_app_mesh_handler;

void mesh_tx_message(message_t* msg);

uint16_t mesh_node_id()
{
    return UICR_NODE_ID;
}

uint8_t mesh_channel()
{
    return UICR_RF_CHANNEL;
}

void mesh_pre_tx()
{
    if(UICR_LISTENING == 0xBABA)
    {
        nrf_esb_stop_rx();
        NRF_LOG_DEBUG("switch to IDLE mode that aloows TX");
    }

}

void mesh_post_tx()
{
    if(UICR_LISTENING == 0xBABA)
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
    if(UICR_is_router())
    {
        uint8_t ttl = msg->control & 0x0F;
        if(ttl>0)
        {
            ttl--;
            msg->control &= 0xF0;//clear ttl
            msg->control |= ttl;
            mesh_tx_message(msg);
        }
    }
    //app would get a damaged ttl, but should not be using it
    if(m_app_mesh_handler != NULL)
    {
        m_app_mesh_handler(msg);
    }
}

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    static uint32_t count = 0;
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
            // Get the most recent element from the RX FIFO.
            while(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {
                mesh_esb_2_message_payload(&rx_payload,&rx_msg);
                NRF_LOG_INFO("ESB Rx %d- pipe: (%d) -> pid:%d ; length:%d",count++,rx_payload.pipe,rx_payload.pid,rx_payload.length);
                NRF_LOG_INFO("HSM - src: (%d) -> pid:0x%02X ; length:%d",rx_msg.source,rx_msg.pid, rx_msg.payload_length);
                mesh_rx_handler(&rx_msg);
            }

            // For each LED, set it as indicated in the rx_payload, but invert it for the button
            // which is pressed. This is because the ack payload from the PRX is reflecting the
            // state from before receiving the packet.
            break;
        default:
            NRF_LOG_ERROR("ESB Unhandled Event (%d)",p_event->evt_id);
            break;
    }

    esb_completed = true;
}


uint32_t mesh_init(app_mesh_handler_t handler)
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

    m_app_mesh_handler = handler;

    nrf_esb_config.retransmit_count         = 0;
    nrf_esb_config.selective_auto_ack       = true;//false is not supported : payload.noack  decides
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.payload_length           = 8;//Not used by DPL as then the MAX is configured
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    if(UICR_LISTENING == 0xBABA)
    {
        nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
    }
    else
    {
        nrf_esb_config.mode                     = NRF_ESB_MODE_PTX;
    }

    //nrf_esb_config.crc                      = NRF_ESB_CRC_16BIT;
    nrf_esb_config.crc                      = NRF_ESB_CRC_OFF;

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

    tx_payload.length  = 8;
    tx_payload.pipe    = 0;
    tx_payload.data[0] = 0x00;

    NRF_LOG_INFO("nodeId %d",UICR_NODE_ID);
    NRF_LOG_INFO("channel %d",UICR_RF_CHANNEL);

    if(UICR_LISTENING == 0xBABA)
    {
        err_code = nrf_esb_start_rx();
        VERIFY_SUCCESS(err_code);
        NRF_LOG_INFO("listening : 0x%X",UICR_LISTENING);
    }
    else
    {
        NRF_LOG_INFO("Not listening : 0x%X",UICR_LISTENING);
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

/**
 * @doxdocgen
 * 
 * @param msg : the message structure to be transmitted 
 */
void mesh_tx_message(message_t* p_msg)
{
    mesh_pre_tx();

    mesh_message_2_esb_payload(p_msg,&tx_payload);

    esb_completed = false;//reset the check
    NRF_LOG_DEBUG("________________TX esb payload length = %d________________",tx_payload.data[0]);
    //should not wait for esb_completed here as does not work from ISR context

    //NRF_ESB_TXMODE_AUTO is used no need to call nrf_esb_start_tx()
    //which could be used for a precise time transmission
    nrf_esb_write_payload(&tx_payload);
    
    //nrf_esb_start_tx();//as the previous mode does not start it due to .mode
}

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

void mesh_tx_reset()
{
    mesh_tx_pid(Mesh_Pid_Reset);
}

void mesh_tx_data(uint8_t pid,uint8_t * data,uint8_t size)
{
    message_t msg;

    msg.control = 0x80 | 2;         // broadcast | ttl = 2
    msg.pid     = pid;
    msg.source  = UICR_NODE_ID;
    msg.payload = data;
    msg.payload_length = size;

    mesh_tx_message(&msg);
}

/**
 * @brief Broadcast an alive packet
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

    mesh_tx_data(Mesh_Pid_Alive,data,5);
    
    return live_count++;//returns the first used value before the increment
}



void mesh_tx_light(uint32_t light)
{
    mesh_tx_data(Mesh_Pid_Light,(uint8_t*)&light,4);
}

void mesh_tx_battery(uint16_t voltage)
{
    uint8_t data[2];
    data[0] = 0xFF & (uint8_t)(voltage >> 8);
    data[1] = 0xFF & (uint8_t)(voltage);
    mesh_tx_data(Mesh_Pid_Battery,data,2);
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
    mesh_tx_data(Mesh_Pid_bme,data,12);
}

int rx_alive(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size != 5)
    {
        return sprintf(p_msg,"size not 5 but:%d",size);
    }
    else
    {
        uint32_t live_count;
        live_count  = data[0] << 24;
        live_count |= data[1] << 16;
        live_count |= data[2] << 8;
        live_count |= data[3];
        uint8_t tx_power = data[4];
        return sprintf(p_msg,";alive:%lu;tx_power:%d",live_count,tx_power);
    }
}

void mesh_parse(message_t* msg,char * p_msg)
{ 
    int add;
    add = sprintf(p_msg,"rssi:-%d;id:%d;ctrl:0x%02X;src:%d",msg->rssi,msg->source,msg->control,msg->source);
    p_msg += add;
    switch(msg->pid)
    {
        case Mesh_Pid_Alive:
            {
                add = rx_alive(p_msg,msg->payload,msg->payload_length);
                p_msg += add;
            }
            break;
        default:
        {
            add = sprintf(p_msg,";pid:0x%02X",msg->pid);
            p_msg += add;
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
