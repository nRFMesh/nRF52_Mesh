
#include "timestamp.h"

#include <stdbool.h>
#include <stdint.h>

#include "sdk_common.h"
#include "sdk_config.h"

#include "boards.h"

#include "nrf_drv_timer.h"

#include "nrf.h"

#include "app_error.h"
#include "app_util.h"

#if(TIMESTAMP_ENABLED != 1)
    #error this file shall only be included if the BLDC is activated
#endif

#if (TIMER_ENABLED != 1)
    #error the timer instance must be enabled for bldc module
#endif

const nrf_drv_timer_t TIMER_TIMESTAMP = NRF_DRV_TIMER_INSTANCE(TIMESTAMP_TIMER_INSTANCE);

void timer_dummy_handler(nrf_timer_event_t event_type, void * p_context)
{
}

void timestamp_init()
{
    ret_code_t err_code;

    nrf_drv_timer_config_t timer_cfg = {
        .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_1MHz,
        .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
        .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_32,
        .interrupt_priority = 7,
        .p_context          = NULL                                                       
    };
    err_code = nrf_drv_timer_init(&TIMER_TIMESTAMP, &timer_cfg, timer_dummy_handler);
    APP_ERROR_CHECK(err_code);

    // Enable timer
    nrf_drv_timer_enable(&TIMER_TIMESTAMP);
}

//timestamp_get() not efficient as triggers a capture event then ready the capture reg, no direct access to counter
uint32_t timestamp_get()
{
    return nrf_drv_timer_capture(&TIMER_TIMESTAMP, 0);//Capture channel 0
}

void timestamp_reset()
{
    return nrf_drv_timer_clear(&TIMER_TIMESTAMP);
}

