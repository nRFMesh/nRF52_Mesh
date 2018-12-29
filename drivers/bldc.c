
#include "bldc.h"

#include <stdbool.h>
#include <stdint.h>

#include "sdk_common.h"
#include "sdk_config.h"

#include "boards.h"

#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"

#include "nrf.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"

#include "app_error.h"
#include "app_util.h"

#if(APP_BLDC_ENABLED != 1)
    #error this file shall only be included if the BLDC is activated
#endif

#if (TIMER_ENABLED != 1)
    #error the timer instance must be enabled for bldc module
#endif

const nrf_drv_timer_t TIMER_BLDC = NRF_DRV_TIMER_INSTANCE(APP_BLDC_TIMER_INSTANCE);

void timer_dummy_handler(nrf_timer_event_t event_type, void * p_context){}


void bldc_events_setup()
{
    uint32_t compare_evt_addr;
    uint32_t gpiote_task_addr;
    nrf_ppi_channel_t ppi_channel;
    ret_code_t err_code;
    nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);

    err_code = nrf_drv_gpiote_out_init(GPIO_M_P1, &config);
    APP_ERROR_CHECK(err_code);


    nrf_drv_timer_extended_compare(&TIMER_BLDC, (nrf_timer_cc_channel_t)0, 200 * 1000UL, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);

    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel);
    APP_ERROR_CHECK(err_code);

    compare_evt_addr = nrf_drv_timer_event_address_get(&TIMER_BLDC, NRF_TIMER_EVENT_COMPARE0);
    gpiote_task_addr = nrf_drv_gpiote_out_task_addr_get(GPIO_M_P1);

    err_code = nrf_drv_ppi_channel_assign(ppi_channel, compare_evt_addr, gpiote_task_addr);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_enable(ppi_channel);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_task_enable(GPIO_M_P1);
}

void bldc_init()
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    err_code = nrf_drv_timer_init(&TIMER_BLDC, &timer_cfg, timer_dummy_handler);
    APP_ERROR_CHECK(err_code);

    // Setup PPI channel with event from TIMER compare and task GPIOTE pin toggle.
    bldc_events_setup();

    // Enable timer
    nrf_drv_timer_enable(&TIMER_BLDC);

}

void bldc_set(float alpha, float norm)
{

}

