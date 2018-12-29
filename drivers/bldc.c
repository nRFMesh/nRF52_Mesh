
#include "bldc.h"

#include <stdbool.h>
#include <stdint.h>

#include "sdk_common.h"
#include "sdk_config.h"

#include "boards.h"

#include "nrf_drv_pwm.h"

#include "nrf.h"
#include "app_error.h"
#include "app_util.h"

#if(APP_BLDC_ENABLED != 1)
    #error this file shall only be included if the BLDC is activated
#endif

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static uint16_t const              pwm_top_period  = 800;//50 us for 16 MHz clock
static nrf_pwm_values_individual_t pwm_values;
static nrf_pwm_sequence_t const    pwm_playback =
{
    .values.p_individual = &pwm_values,
    .length              = NRF_PWM_VALUES_LENGTH(pwm_values),
    .repeats             = 0,
    .end_delay           = 0
};

static void demo1_handler(nrf_drv_pwm_evt_type_t event_type)
{
    /*
    if (event_type == NRF_DRV_PWM_EVT_FINISHED)
    {
        uint8_t channel    = m_demo1_phase >> 1;
        bool    down       = m_demo1_phase & 1;
        bool    next_phase = false;

        uint16_t * p_channels = (uint16_t *)&pwm_values;
        uint16_t value = p_channels[channel];
        if (down)
        {
            value -= m_demo1_step;
            if (value == 0)
            {
                next_phase = true;
            }
        }
        else
        {
            value += m_demo1_step;
            if (value >= m_demo1_top)
            {
                next_phase = true;
            }
        }
        p_channels[channel] = value;

        if (next_phase)
        {
            if (++m_demo1_phase >= 2 * NRF_PWM_CHANNEL_COUNT)
            {
                m_demo1_phase = 0;
            }
        }
    }
    */
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
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, demo1_handler));

    pwm_values.channel_0 = 50  | 0x8000;
    pwm_values.channel_1 = 200 | 0x8000;
    pwm_values.channel_2 = 400 | 0x8000;
    pwm_values.channel_3 = 0   | 0x8000;

    (void)nrf_drv_pwm_simple_playback(&m_pwm0, &pwm_playback, 1,NRF_DRV_PWM_FLAG_LOOP);

}

void bldc_set(float alpha, float norm)
{
    pwm_values.channel_0 = (uint16_t) alpha | 0x8000;//polarity
}

