
#include "compare.h"

#include <stdbool.h>
#include <stdint.h>

#include "sdk_common.h"
#include "sdk_config.h"

#include "boards.h"

#include "nrf_drv_timer.h"

#include "nrf.h"

#include "app_error.h"
#include "app_util.h"

#if (TIMER_ENABLED != 1)
    #error the timer instance must be enabled for bldc module
#endif

const nrf_drv_timer_t TIMER_COMPARE = NRF_DRV_TIMER_INSTANCE(COMPARE_TIMER_INSTANCE);

apptimer_config_t m_config;

void timer_compare_handler(nrf_timer_event_t event_type, void * p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
        {
            m_config.call0();
        }
        break;
        case NRF_TIMER_EVENT_COMPARE1:
        {
            m_config.call1();
        }
        break;
        default:
            //Do nothing.
            break;
    }
}

void compare_init(apptimer_config_t config)
{
    ret_code_t err_code;

    nrf_drv_timer_config_t timer_cfg = {
        .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_1MHz,
        .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
        .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_32,
        .interrupt_priority = 7,
        .p_context          = NULL                                                       
    };
    err_code = nrf_drv_timer_init(&TIMER_COMPARE, &timer_cfg, timer_compare_handler);
    APP_ERROR_CHECK(err_code);

    m_config = config;
    nrf_drv_timer_compare(&TIMER_COMPARE,0,m_config.offset0,true);
    nrf_drv_timer_compare(&TIMER_COMPARE,1,m_config.offset1,true);
    nrf_drv_timer_compare(&TIMER_COMPARE,5,m_config.cycle,true);

    // Enable timer
    nrf_drv_timer_enable(&TIMER_COMPARE);
}
