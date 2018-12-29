#ifndef __APP_BLDC_H__
#define __APP_BLDC_H__


#include <stdint.h>

void bldc_init();
void bldc_set(int angle, float norm);

#endif /*__APP_BLDC_H__*/
