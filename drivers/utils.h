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

int sprint_buf(char*str,const char*msg,uint8_t size);

uint8_t strbegins (const char * s1, const char * s2);

uint8_t text2bin(const char*text,uint8_t length,uint8_t*data,uint8_t*size);

#endif /*__UTILS_H__*/