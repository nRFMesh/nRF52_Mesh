
#include "clocks.h"

#include "sdk_common.h"
#include "boards.h"

//attempt to move to nrfx but nrf_drv_clock_init() do not have application examples,...
#include "nrf_drv_clock.h"
#include "nrf_drv_rtc.h"

#define NRF_LOG_MODULE_NAME clocks

#if (CLOCKS_CONFIG_LOG_ENABLED == 1)
#define NRF_LOG_LEVEL CLOCKS_CONFIG_LOG_LEVEL
#else //CLOCKS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif //CLOCKS_CONFIG_LOG_ENABLED

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define UICR_SLEEP_SEC     NRF_UICR->CUSTOMER[2]


const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

static app_rtc_handler_t m_app_rtc_handler;

void clocks_start( void )
{
    ret_code_t err_code;

    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_hfclk_request(NULL);
    while(!nrf_drv_clock_hfclk_is_running());
    
}

void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_COMPARE0)
    {
        nrf_drv_rtc_counter_clear(&rtc);
        nrf_drv_rtc_int_enable(&rtc, NRF_RTC_INT_COMPARE0_MASK);
        NRF_LOG_DEBUG("rtc_handler(COMPARE)");

        static uint8_t send = 1;
        NRF_LOG_INFO("rtc_handler() %d",send);
        send++;

        m_app_rtc_handler();

    }
}

void rtc_config(app_rtc_handler_t handler)
{
    uint32_t err_code;

    m_app_rtc_handler = handler;

    nrf_drv_clock_lfclk_request(NULL);

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = 4095;
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Enable tick event & interrupt
    //nrf_drv_rtc_tick_enable(&rtc,true);

    //Set compare channel to trigger interrupt after COMPARE_COUNTERTIME seconds
    err_code = nrf_drv_rtc_cc_set(&rtc,0,UICR_SLEEP_SEC * 8,true);
    APP_ERROR_CHECK(err_code);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

