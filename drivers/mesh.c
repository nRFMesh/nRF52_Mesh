
#include "mesh.h"

#include "sdk_common.h"

#include "boards.h"

#include "nrf_esb.h"
#include "nrf_esb_error_codes.h"

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
//light was 16bit redefined u32 bit
#define Mesh_Pid_Light 0x07
//bme280 no more with uncalibrated params
#define Mesh_Pid_bme 0x0A

#define UICR_NODE_ID     NRF_UICR->CUSTOMER[0]
#define UICR_RF_CHANNEL NRF_UICR->CUSTOMER[1]

static nrf_esb_payload_t tx_payload = NRF_ESB_CREATE_PAYLOAD(0, 0x01, 0x00);
static nrf_esb_payload_t rx_payload;
static volatile bool esb_completed = false;


void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            (void) nrf_esb_flush_tx();
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            // Get the most recent element from the RX FIFO.
            while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS) ;

            // For each LED, set it as indicated in the rx_payload, but invert it for the button
            // which is pressed. This is because the ack payload from the PRX is reflecting the
            // state from before receiving the packet.
            break;
    }

    esb_completed = true;
}


uint32_t mesh_init()
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;

    nrf_esb_config.retransmit_count         = 0;
    nrf_esb_config.selective_auto_ack       = true;//payload.noack  decides
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.payload_length           = 8;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PTX;
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

    return NRF_SUCCESS;
}

uint32_t mesh_tx_button(uint8_t state)
{
    uint32_t err_code;
    tx_payload.length   = 3;//payload + header (crc length not included)
    tx_payload.control = 0x80 | 2;// broadcast | ttl = 2
    tx_payload.noack    = true;//it is a broadcast
    tx_payload.pipe     = 0;
    
    tx_payload.data[0] = 0x06;//pid
    tx_payload.data[1] = UICR_NODE_ID;//source - on_off_tag
    tx_payload.data[2] = state;//Up or Down
    
    tx_payload.noack = true;
    esb_completed = false;
    err_code = nrf_esb_write_payload(&tx_payload);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t mesh_tx_light_on()
{
    uint32_t err_code;
    tx_payload.length   = 4;//payload + header (crc length not included)
    tx_payload.control = 0x7B;// light
    tx_payload.noack    = true;//it is a broadcast
    tx_payload.pipe     = 0;
    
    tx_payload.data[0] = UICR_NODE_ID;//source
    tx_payload.data[1] = 0x19;//dest
    tx_payload.data[2] = 0xA0;//msb
    tx_payload.data[3] = 0x00;//lsb
    
    tx_payload.noack = true;
    esb_completed = false;
    err_code = nrf_esb_write_payload(&tx_payload);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t mesh_tx_light_off()
{
    uint32_t err_code;
    tx_payload.length   = 4;//payload + header (crc length not included)
    tx_payload.control = 0x7B;// light
    tx_payload.noack    = true;//it is a broadcast
    tx_payload.pipe     = 0;
    
    tx_payload.data[0] = UICR_NODE_ID;//source
    tx_payload.data[1] = 0x19;//dest
    tx_payload.data[2] = 0x00;//msb
    tx_payload.data[3] = 0x00;//lsb
    
    tx_payload.noack = true;
    esb_completed = false;
    err_code = nrf_esb_write_payload(&tx_payload);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

void mesh_wait_tx()
{
    while(!esb_completed);
}

void mesh_tx_pid(uint8_t pid)
{
    esb_completed = false;//reset the check

    tx_payload.length   = 2;//payload + header (crc length not included)
    tx_payload.control = 0x80 | 2;// broadcast | ttl = 2
    tx_payload.noack    = true;//it is a broadcast
    tx_payload.pipe     = 0;

    tx_payload.data[0] = pid;
    
    tx_payload.data[1] = UICR_NODE_ID;//source
    
    tx_payload.noack = true;
    nrf_esb_write_payload(&tx_payload);

    //wait till the transmission is complete
    while(!esb_completed);
}

void mesh_tx_reset()
{
    mesh_tx_pid(Mesh_Pid_Reset);
}

void mesh_tx_alive()
{
    mesh_tx_pid(Mesh_Pid_Alive);
}

void mesh_tx_data(uint8_t pid,uint8_t * data,uint8_t size)
{
    esb_completed = false;//reset the check

    tx_payload.length   = 2 + size;//length,ctrl payload(pid,source,rf_payload)(crc not included in length)
    tx_payload.control = 0x80 | 2;// broadcast | ttl = 2
    tx_payload.noack    = true;//it is a broadcast
    tx_payload.pipe     = 0;

    tx_payload.data[0] = pid;//acceleration
    
    tx_payload.data[1] = UICR_NODE_ID;//source

    for(int i=0;i<size;i++)
    {
        tx_payload.data[i+2]     = data[i];
    }
    
    tx_payload.noack = true;
    nrf_esb_write_payload(&tx_payload);

    //wait till the transmission is complete
    while(!esb_completed);
}

void mesh_tx_light(uint32_t light)
{
    mesh_tx_data(Mesh_Pid_Light,(uint8_t*)&light,4);
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