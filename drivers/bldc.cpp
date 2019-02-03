
#include "bldc.hpp"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "sdk_common.h"
#include "sdk_config.h"
#include "boards.h"

#include "nrf.h"
#include "app_error.h"
#include "app_util.h"

#include "nrfx_pwm.h"


static uint16_t const              pwm_top_period  = 400;//25 us for 16 MHz clock
static nrf_pwm_values_individual_t pwm_values;
nrf_pwm_sequence_t pwm_playback;

#define M_PI    3.14159265358979323846
#define M_2xPI  6.28318530717958647692

uint16_t sin_Table[256];

typedef struct{
    //bool is_enabled;
    float   norm;
    float   rot_per_sec,steps_per_100_us;
    int     nb_poles;
    float absolute_steps;
    float absolute_target;
    bool  is_tracking;
}bldc_status_t;

bldc_status_t motor;

//called every 25 us
static void pwm_position_handler(nrfx_pwm_evt_type_t event_type)
{
    static uint32_t count = 0;
    if(motor.is_tracking)
    {
        //executes every 100 us
        if(count%4 == 0)
        {
            float diff = motor.absolute_target - motor.absolute_steps;
            if(abs(diff)>0.01)
            {
                if(motor.absolute_steps < motor.absolute_target)
                {
                    motor.absolute_steps += motor.steps_per_100_us;
                }
                else
                {
                    motor.absolute_steps -= motor.steps_per_100_us;
                }
                // ~ 3 us
                bldc_set_pole(motor.absolute_steps, motor.norm);
            }
        }
        count++;
    }
}

NRF_PWM_Type* pwm_base_addresses[4]={(NRF_PWM_Type*)NRF_PWM0_BASE, (NRF_PWM_Type*)NRF_PWM1_BASE, (NRF_PWM_Type*)NRF_PWM2_BASE, (NRF_PWM_Type*)NRF_PWM3_BASE};

void bldc_init(uint8_t pwm,uint8_t p1, uint8_t p2, uint8_t p3)
{
    nrfx_pwm_t pwm_instance = {pwm_base_addresses[pwm], pwm};//{p_registers,drv_inst_idx}

    nrfx_pwm_config_t const config0 =
    {
        .output_pins =
        {
            p1, // channel 0
            p2, // channel 1
            p3, // channel 2
            NRFX_PWM_PIN_NOT_USED  // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_16MHz,
        .count_mode   = NRF_PWM_MODE_UP_AND_DOWN,//NRF_PWM_MODE_UP_AND_DOWN,NRF_PWM_MODE_UP
        .top_value    = pwm_top_period,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    nrfx_pwm_init(&pwm_instance, &config0, pwm_position_handler);


    pwm_playback.values.p_individual = &pwm_values;
    pwm_playback.length              = NRF_PWM_VALUES_LENGTH(pwm_values);
    pwm_playback.repeats             = 0;
    pwm_playback.end_delay           = 0;


    (void)nrfx_pwm_simple_playback(&pwm_instance, &pwm_playback, 1,NRFX_PWM_FLAG_LOOP);

    for(int i=0;i<256;i++)
    {
            float angle = i * 2*M_PI /256;
            sin_Table[i] = pwm_top_period * (sin(angle)+1) / 2;
    }


    motor.nb_poles = 14;
    motor.is_tracking = false;
    motor.absolute_steps = 0;
    motor.absolute_target   = 0;
    motor.norm = 0;
    motor.rot_per_sec = 0;
    motor.steps_per_100_us = 0;

    bldc_set_pole(motor.absolute_steps, motor.norm);
}

void bldc_set_target(int32_t absolute_steps)
{
    motor.absolute_target = absolute_steps;
    motor.is_tracking = true;
}

void bldc_set_speed(float v_rot_per_sec)
{
    motor.rot_per_sec = v_rot_per_sec;
    motor.steps_per_100_us = motor.nb_poles * motor.rot_per_sec * (256.0/10000.0);
}

void bldc_set_norm(float norm)
{
    motor.norm = norm;
}

void bldc_set_pole(int angle, float norm)
{
    int a1 = angle % 256;
    uint16_t pwm1_buf = norm * sin_Table[a1];
    int a2 = a1 + 85;
    if(a2>=256)a2-=256;
    uint16_t pwm2_buf = norm * sin_Table[a2];
    int a3 = a1 + 170;
    if(a3>=256)a3-=256;
    uint16_t pwm3_buf = norm * sin_Table[a3];

    pwm_values.channel_0 = pwm1_buf | 0x8000;
    pwm_values.channel_1 = pwm2_buf | 0x8000;
    pwm_values.channel_2 = pwm3_buf | 0x8000;
}

void bldc_pwm_get(uint16_t *pwm1,uint16_t *pwm2,uint16_t *pwm3)
{
    *pwm1 = pwm_values.channel_0 & ~0x8000;
    *pwm2 = pwm_values.channel_1 & ~0x8000;
    *pwm3 = pwm_values.channel_2 & ~0x8000;
}