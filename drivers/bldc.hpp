#ifndef _APP_BLDC_HPP__
#define _APP_BLDC_HPP__


#include <stdint.h>
#include "nrf_pwm.h"

class bldc_c
{
    public:
        bldc_c(uint8_t pwm,uint8_t p1, uint8_t p2, uint8_t p3);
        void set_target(int32_t absolute_steps);
        void set_speed(float v_rot_per_sec);
        void set_norm(float norm);
        void set_pole(int angle);
        void get_pwm(uint16_t *pwm1,uint16_t *pwm2,uint16_t *pwm3);

    public:
        float   norm;
        float   rot_per_sec,steps_per_100_us;
        int     nb_poles;
        float absolute_steps;
        float absolute_target;
        bool  is_tracking;
        nrf_pwm_values_individual_t pwm_values;
};



#endif /*_APP_BLDC_HPP__*/
