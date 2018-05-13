#ifndef __MAX_44009_H__
#define __MAX_44009_H__
/** @file max44009.h
 *
 * @author Wassim FILALI
 *
 * @compiler IAR STM8 -> gcc
 *
 *
 * $Date: 29.10.2016 - creation out of refactoring
 * $Data: 06.05.2018 - port to nRF52 with nRF SDK 15 and gcc
 * $Revision: 2
 *
*/

#include <stdint.h>

#include "nrf_drv_twi.h"

//read the light in mili-lux => x1000
//resolution is 0.045 @ [0-11.5] ---> 737 @ [94K->188K]
//0.05–0.3	Full moon on a clear night
//10,000–25,000	Full daylight (not direct sun)
//32,000–100,000	Direct sunlight
uint32_t max44009_read_light();

void max44009_test();

#endif /*__MAX_44009_H__*/
