#ifndef __MESH_H__
#define __MESH_H__


#include <stdint.h>
#include <stdbool.h>

//should match NRF_ESB_MAX_PAYLOAD_LENGTH
#define MAX_MESH_MESSAGE_SIZE (NRF_ESB_MAX_PAYLOAD_LENGTH-4)

typedef struct 
{
    uint8_t control;
    uint8_t pid;
    uint8_t source;
    uint8_t dest;
    int8_t  rssi;       //Radio Signal Strength Indication
    uint8_t payload_length;
    uint8_t *payload;
}message_t;

extern const char * const pid_name[];

//------------------------- Mesh Functions Identifiers -------------------------

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
#define Mesh_Pid_Text       0x16
#define Mesh_Pid_MQTT       0x17
#define Mesh_Pid_ExecuteCmd 0xEC

#define MESH_Broadcast_Header_Length 4
#define MESH_P2P_Header_Length 5

#define MESH_cmd_node_id_set        0x01
#define MESH_cmd_node_id_get        0x02
#define MESH_cmd_rf_chan_set        0x03
#define MESH_cmd_rf_chan_get        0x04
#define MESH_cmd_tx_power_set       0x05
#define MESH_cmd_tx_power_get       0x06
#define MESH_cmd_bitrate_set        0x07
#define MESH_cmd_bitrate_get        0x08
#define MESH_cmd_crc_set            0x09
#define MESH_cmd_crc_get            0x0A

//------------------------- Mesh Macros -------------------------

#define MESH_IS_BROADCAST(val) ((val & 0x80) == 0x80)
#define MESH_IS_PEER2PEER(val) ((val & 0x80) == 0x00)
//Ack if bits 1,2 == 1,0 => MASK 0x60, VAL 0x40
#define MESH_IS_ACKNOWLEDGE(val) ((val & 0x60) == 0x40)
#define MESH_WANT_ACKNOWLEDGE(val) ((val & 0xF0) == 0x70)
#define MESH_IS_RESPONSE(val) ((val & 0xF0) == 0x00)


//------------------------- Mesh Core -------------------------

typedef void (*app_mesh_rf_handler_t)(message_t*);

typedef void (*app_mesh_cmd_handler_t)(const char*,uint8_t);

uint8_t mesh_channel();

uint32_t mesh_init(app_mesh_rf_handler_t rf_handler,app_mesh_cmd_handler_t cmd_handler);

void mesh_execute_cmd(uint8_t*data,uint8_t size,bool is_rf_request,uint8_t rf_nodeid);

//------------------------- Mesh protocol -------------------------

void mesh_consume_rx_messages();

void mesh_wait_tx();

void mesh_tx_reset();
uint32_t mesh_tx_alive();

void mesh_bcast_data(uint8_t pid,uint8_t * data,uint8_t size);
void mesh_bcast_text(char *text);
void mesh_ttl_set(uint8_t ttl);

uint32_t mesh_tx_button(uint8_t state);
void mesh_tx_light(uint32_t light);
void mesh_tx_battery(uint16_t voltage);
void mesh_tx_bme(int32_t temp,uint32_t hum,uint32_t press);

void mesh_parse(message_t* msg,char * p_msg);
void mesh_parse_raw(message_t* msg,char * p_msg);
void mesh_parse_bytes(message_t* msg,char * p_msg);

//------------------------- Mesh Commander -------------------------

void mesh_text_request(const char*text,uint8_t length);

//-------------------------- Parsers -------------------------------
int rx_accell(char * p_msg,uint8_t*data,uint8_t size);
int rx_gyro(char * p_msg,uint8_t*data,uint8_t size);


#endif /*__MESH_H__*/
