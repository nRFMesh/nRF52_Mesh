#ifndef _APP_BLDC_HPP__
#define _APP_BLDC_HPP__


#include <stdint.h>

void bldc_init(uint8_t pwm,uint8_t p1, uint8_t p2, uint8_t p3);

void bldc_set_target(int32_t absolute_steps);
void bldc_set_speed(float v_rot_per_sec);
void bldc_set_norm(float norm);
void bldc_set_pole(int angle, float norm);
void bldc_pwm_get(uint16_t *pwm1,uint16_t *pwm2,uint16_t *pwm3);

#endif /*_APP_BLDC_HPP__*/
