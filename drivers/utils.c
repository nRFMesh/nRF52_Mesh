/**
 * @file utils.c
 * @author Wassim Filali
 * @date 01 June 2018
 * @brief File containing utilities for buffers and string manipulations
 *
 * 
 */

#include "utils.h"
#include <stdio.h>

#include "boards.h"
#include "nrf_delay.h"

#include "uicr_user_defines.h"


uint16_t get_this_node_id()
{
    return UICR_NODE_ID;
}


/**
 * @brief prints a buffer in hex string
 * 
 * @param str the output hex text
 * @param msg the binary buffer 
 * @param size the number of bytes to be considered from msg
 * @return int the number of characters printed in str
 */
int sprint_buf(char*str,const char*msg,uint8_t size)
{
    int total=0;
    int add;
    add = sprintf(str,"0x");
    str+=add;
    total+=add;
    for(int i=0;i<size;i++)
    {
        add = sprintf(str,"%02X",msg[i]);
        str+=add;
        total+=add;
    }
    return total;
}

/**
 * @brief This function does not use strlen which makes it more efficient
 * 
 * @param s1 the buffer to be checked
 * @param s2 the string that could ba a const text e.g. "msg:"
 * @return uint8_t 1 for yes begins , 0 for no match
 */
uint8_t strbegins (const char * s1, const char * s2)
{
    for(; *s1 == *s2; ++s1, ++s2)
        if(*s2 == 0)
            return 1;
    return (*s2 == 0)?1:0;
}

/**
 * @brief Turns an ASCII char into a byte
 * 
 * @param c ASCII char '0'..'9' , 'A'..'F', 'a'..'f'
 * @return uint8_t the deciman value of the ASCII char
 */
uint8_t get_hex_char(char c)
{
	uint8_t res = 0;
	if(c <= '9')
	{
		res = c - '0';
	}
	else if(c <= 'F')
	{
		res = c - 'A' + 10;
	}
	else if(c <= 'f')
	{
		res = c - 'a' + 10;
	}
	return res;
}

/**
 * @brief reads a text hex that is exactly 2 characters and 
 * returs its decimal ASCII value
 * 
 * @param buffer text hex e.g. "FE", "5A", "3b", "20", "0F"
 * @return uint8_t the decimal value [0..255]
 */
uint8_t get_hex(const char* buffer)
{
	uint8_t hex_val;
	hex_val = get_hex_char(*buffer);
	hex_val <<= 4;
	hex_val |= get_hex_char(*(buffer+1));
	return hex_val;
}


/**
 * @brief converts a hex text data such as "0x0011223344" into
 * binary data [0x00, 0x11, 0x22, 0x33, 0x44] ignoring the first
 * leading 0x and providing the size which is the number of bytes
 * written
 * 
 * @param text a text string starting with 0x and having exactly 2 characters per byte
 * @param length >= 4, there has to be at least 1 hex char "0x12" whic is 4 characters long
 * @param data output binary data
 * @param size the size of the output binary data
 * @return 1 on success, 0 on failure
 */
uint8_t text2bin(const char*text,uint8_t length,uint8_t*data,uint8_t*size)
{
    if((length < 4) || ((length % 2)!= 0) )
    {
        return 0;
    }
    if( (text[0] != '0') || (text[1] != 'x') )
    {
        return 0;
    }
    (*size) = 0;
    const char *p_text = text+2;
    for(int i=2;i<length;i+=2)
    {
        *data=get_hex(p_text);
        p_text+=2;
        data++;//incremented but will not be modified for the user
        (*size)++;//incremeneted to inform the user of the size
    }
    return 1;
}



void blink_green(int time,int afteroff)
{
    led2_green_on();
    nrf_delay_ms(time);
    led2_green_off();
    if(afteroff)
    {
        nrf_delay_ms(afteroff);
    }
}

void blink_red(int time,int afteroff)
{
    led1_red_on();
    nrf_delay_ms(time);
    led1_red_off();
    if(afteroff)
    {
        nrf_delay_ms(afteroff);
    }
}

void blink_blue(int time,int afteroff)
{
    led1_blue_on();
    nrf_delay_ms(time);
    led1_blue_off();
    if(afteroff)
    {
        nrf_delay_ms(afteroff);
    }
}
