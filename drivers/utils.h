/**
 * @file utils.h
 * @author Wassim Filali
 * @date 01 June 2018
 * @brief File containing utilities for buffers and string manipulations
 *
 * 
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

#define DEBUGPIN_RF     14
#define DEBUGPIN_UART   29

#define APP_DEBUG_PIN_SET(a)    (NRF_GPIO->OUTSET = (1 << (a))) //!< Used internally to set debug pins.
#define APP_DEBUG_PIN_CLR(a)    (NRF_GPIO->OUTCLR = (1 << (a))) //!< Used internally to clear debug pins.


uint16_t get_this_node_id();

int sprint_buf(char*str,const char*msg,uint8_t size);

uint8_t strbegins (const char * s1, const char * s2);

uint8_t text2bin(const char*text,uint8_t length,uint8_t*data,uint8_t*size);

void blink_green(int time,int afteroff);
void blink_red(int time,int afteroff);
void blink_blue(int time,int afteroff);

#if defined(BOARD_PCA10059)
    #define led2_green_on()   bsp_board_led_on(0)
    #define led2_green_off()  bsp_board_led_off(0)
    #define led1_red_on()     bsp_board_led_on(1)
    #define led1_red_off()    bsp_board_led_off(1)
    #define led1_green_on()    bsp_board_led_on(2)
    #define led1_green_off()   bsp_board_led_off(2)
    #define led1_blue_on()   bsp_board_led_on(3)
    #define led1_blue_off()  bsp_board_led_off(3)
#else
    #define led2_green_on()  
    #define led2_green_off() 
    #define led1_red_on()    
    #define led1_red_off()   
    #define led1_green_on()  
    #define led1_green_off() 
    #define led1_blue_on()   
    #define led1_blue_off()  
#endif

#endif /*__UTILS_H__*/