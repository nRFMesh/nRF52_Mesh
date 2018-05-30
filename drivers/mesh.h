#ifndef __MESH_H__
#define __MESH_H__


#include <stdint.h>

//should match NRF_ESB_MAX_PAYLOAD_LENGTH
#define MAX_MESH_MESSAGE_SIZE 32

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

typedef void (*app_mesh_handler_t)(message_t*);

uint16_t mesh_node_id();
uint8_t mesh_channel();

uint32_t mesh_init(app_mesh_handler_t handler);

void mesh_wait_tx();

void mesh_tx_reset();
uint32_t mesh_tx_alive();

void mesh_tx_data(uint8_t pid,uint8_t * data,uint8_t size);

uint32_t mesh_tx_button(uint8_t state);
void mesh_tx_light(uint32_t light);
void mesh_tx_battery(uint16_t voltage);
void mesh_tx_bme(int32_t temp,uint32_t hum,uint32_t press);

void mesh_parse(message_t* msg,char * p_msg);
void mesh_parse_raw(message_t* msg,char * p_msg);
void mesh_parse_bytes(message_t* msg,char * p_msg);

#endif /*__MESH_H__*/
