
#include "bldc.h"

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "sdk_common.h"
#include "sdk_config.h"

#include "boards.h"

#include "nrf_drv_pwm.h"

#include "nrf.h"
#include "app_error.h"
#include "app_util.h"

#if(BLDC_ENABLED != 1)
    #error this file shall only be included if the BLDC is activated
#endif

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(BLDC_PWM_INSTANCE);

static uint16_t const              pwm_top_period  = 400;//25 us for 16 MHz clock
static nrf_pwm_values_individual_t pwm_values;
static nrf_pwm_sequence_t const    pwm_playback =
{
    .values.p_individual = &pwm_values,
    .length              = NRF_PWM_VALUES_LENGTH(pwm_values),
    .repeats             = 0,
    .end_delay           = 0
};

#define M_PI 3.14159265358979323846

uint16_t sin_Table[256];

static void pwm_dummy_handler(nrf_drv_pwm_evt_type_t event_type)
{
}

void bldc_init()
{
    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            GPIO_M_P1, // channel 0
            GPIO_M_P2, // channel 1
            GPIO_M_P3, // channel 2
            NRFX_PWM_PIN_NOT_USED  // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_16MHz,
        .count_mode   = NRF_PWM_MODE_UP_AND_DOWN,//NRF_PWM_MODE_UP_AND_DOWN,NRF_PWM_MODE_UP
        .top_value    = pwm_top_period,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, pwm_dummy_handler));

    (void)nrf_drv_pwm_simple_playback(&m_pwm0, &pwm_playback, 1,NRF_DRV_PWM_FLAG_LOOP);

    for(int i=0;i<256;i++)
    {
            float angle = i * 2*M_PI /256;
            sin_Table[i] = pwm_top_period * (sin(angle)+1) / 2;
    }

    bldc_set(0, 1.0);
}

void bldc_set(int angle, float norm)
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