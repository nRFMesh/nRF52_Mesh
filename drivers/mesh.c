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

#include "boards.h"

#include "nrf_esb.h"
#include "nrf_esb_error_codes.h"

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


#define Mesh_Pid_Alive 0x05
#define Mesh_Pid_Reset 0x04
#define Mesh_Pid_Button 0x06

//light was 16bit redefined u32 bit
#define Mesh_Pid_Light 0x07
#define Mesh_Pid_Temperature 0x08
//bme280 no more with uncalibrated params
#define Mesh_Pid_bme        0x0A
//following rgb is led color control
#define Mesh_Pid_led_rgb    0x0B
#define Mesh_Pid_light_rgb  0x0E
#define Mesh_Pid_Humidity   0x11
#define Mesh_Pid_Pressure   0x12
#define Mesh_Pid_accell     0x13
#define Mesh_Pid_new_light  0x14
#define Mesh_Pid_Battery    0x15

#define MESH_IS_BROADCAST(val) ((val & 0x80) == 0x80)
//Ack if bits 1,2 == 1,0 => MASK 0x60, VAL 0x40
#define MESH_IS_ACKNOWLEDGE(val) ((val & 0x60) == 0x40)

#define MESH_Broadcast_Header_Length 4
#define MESH_P2P_Header_Length 5

#define MESH_cmd_rf_chan_set        0x01
#define MESH_cmd_rf_chan_get        0x02
#define MESH_cmd_tx_power_set       0x03
#define MESH_cmd_tx_power_get       0x04
#define MESH_cmd_bitrate_set        0x05
#define MESH_cmd_bitrate_get        0x06
#define MESH_cmd_crc_set            0x07
#define MESH_cmd_crc_get            0x08


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

static nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
static nrf_esb_payload_t tx_payload = NRF_ESB_CREATE_PAYLOAD(0, 0x01, 0x00);
static nrf_esb_payload_t rx_payload;
static message_t rx_msg;
static volatile bool esb_completed = false;
static volatile bool esb_tx_complete = false;
static volatile bool esb_rx_complete = true;//on startup no pending rx

static app_mesh_rf_handler_t m_app_rf_handler;

static app_mesh_cmd_handler_t m_app_cmd_handler;

void mesh_tx_message(message_t* msg);



//-------------------------------------------------------------
//------------------------- Mesh Core -------------------------
//-------------------------------------------------------------




uint16_t mesh_node_id()
{
    return UICR_NODE_ID;
}

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
    uint8_t ctrl_backup = msg->control;

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
    
    if(m_app_rf_handler != NULL)
    {
        
        msg->control = ctrl_backup;
        m_app_rf_handler(msg);
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
            //TODO unsafe mutex, should disable interrupts between get and set
            if(esb_rx_complete)//otherwise re-entrant will be handled in the while loop
            {
                esb_rx_complete = false;
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
                esb_rx_complete = true;
            }
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

    tx_payload.length  = 8;
    tx_payload.pipe    = 0;
    tx_payload.data[0] = 0x00;

    NRF_LOG_INFO("nodeId %d",UICR_NODE_ID);
    NRF_LOG_INFO("channel %d",UICR_RF_CHANNEL);

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
 * @brief Transmits a message structure
 * 
 * @param msg : the message structure to be converted to an esb buffer and sent through nrf_esb_write_payload
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

//----------------------------------------------------------------------------
//------------------------- Messages Deserialisation -------------------------
//----------------------------------------------------------------------------


int rx_alive(char * p_msg,uint8_t*data,uint8_t size)
{
    if(size == 0)
    {
        return sprintf(p_msg,"alive:1");
    }
    else if(size == 5)
    {
        uint32_t live_count;
        live_count  = data[0] << 24;
        live_count |= data[1] << 16;
        live_count |= data[2] << 8;
        live_count |= data[3];
        uint8_t tx_power = data[4];
        return sprintf(p_msg,"alive:%lu;tx_power:%d",live_count,tx_power);
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
        return sprintf(p_msg,"light:%lu",light);
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

void mesh_parse(message_t* msg,char * p_msg)
{ 
    p_msg += sprintf(  p_msg,"rssi:-%d;id:%d;ctrl:0x%02X;src:%d;",msg->rssi,msg->pid,msg->control,msg->source);
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
                p_msg += rx_alive(p_msg,msg->payload,msg->payload_length);
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


/**
 * @brief Executes a command received in a binary format
 * 
 * @param data the array starting with <cmd_id> followed by <param0><param1>,...
 * @param size the total size including the first byte of cmd_id
 */
void mesh_execute_cmd(uint8_t*data,uint8_t size)
{
    char resp[32];
    uint8_t resp_len;
    switch(data[0])
    {
        case MESH_cmd_rf_chan_set:
        {
            mesh_wait_tx();//in case any action was ongoing
            nrf_esb_stop_rx();
            nrf_esb_set_rf_channel(data[1]);
            nrf_esb_start_rx();
            resp_len = sprintf(resp,"cmd:set_channel;set:%u;get:%u",data[1],mesh_get_channel());
        }
        break;
        case MESH_cmd_rf_chan_get:
        {
            resp_len = sprintf(resp,"cmd:get_channel;channel:%u",mesh_get_channel());
        }
        break;
        case MESH_cmd_crc_set:
        {
            mesh_wait_tx();//in case any action was ongoing
            nrf_esb_stop_rx();
            mesh_set_crc(data[1]);
            nrf_esb_start_rx();
            resp_len = sprintf(resp,"cmd:set_crc;set:%u;get:%u",data[1],mesh_get_crc());
        }
        break;
        case MESH_cmd_crc_get:
        {
            resp_len = sprintf(resp,"cmd:get_crc;crc:%u",mesh_get_crc());
        }
        break;
        default:
        {
            resp_len = sprintf(resp,"cmd:0x%02X;len:%d",data[0],size);
        }
        break;
    }
    m_app_cmd_handler(resp,resp_len);
}

/**
 * @brief executes teh command immidiatly
 * Future extension should use a command fifo
 * 
 * @param text contains a command line to execute a mesh rf function
 * supported commands :
 * * msg:0x00112233445566...
 * where 0:length, 1:control , 2:source , 3:dest/payload , 4:payload,...
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
            char resp[32];
            uint8_t resp_len = sprintf(resp,"msg_len:%d",size);
            m_app_cmd_handler(resp,resp_len);
        }
    }
    else if(strbegins(text,"cmd:"))
    {
        uint8_t data[32];//TODO define global max cmd size
        uint8_t size;
        if(text2bin(text+4,length-4,data,&size))
        {
            mesh_execute_cmd(data,size);
        }
    }
}
