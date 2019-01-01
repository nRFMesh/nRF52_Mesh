#ifndef __APP_BLDC_H__
#define __APP_BLDC_H__


#include <stdint.h>

void bldc_init();
void bldc_set(int angle, float norm);
void bldc_pwm_get(uint16_t *pwm1,uint16_t *pwm2,uint16_t *pwm3);

#endif /*__APP_BLDC_H__*/
