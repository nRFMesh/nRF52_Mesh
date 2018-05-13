/**
 * Home Smart Mesh
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_util.h"

//for the log
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// --------------------- inputs from sdk_config --------------------- 
// ---> TWI0_ENABLED ---> TWI1_ENABLED

//drivers
#include "bme280.h"
#include "max44009.h"
//apps
#include "clocks.h"
#include "battery.h"
#include "twi.h"
#include "mesh.h"


/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

void read_send_bme()
{
    bme280_measure();

    int32_t temp    = bme280_get_temperature();
    uint32_t hum    = bme280_get_humidity();
    uint32_t press  = bme280_get_pressure();

    mesh_tx_bme(temp,hum,press);

    #if(NRF_LOG_LEVEL <= NRF_LOG_LEVEL_INFO)
        float temperature = (float)temp;
        NRF_LOG_INFO("temperature = "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(temperature/100));
        float humidity = (float)hum;
        NRF_LOG_INFO("humidity = "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(humidity/1024));
        float pressure = (float)press;
        NRF_LOG_INFO("pressure = "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(pressure/(256*100)));
        NRF_LOG_RAW_INFO("\r");
    #endif
}

void read_send_light()
{
    uint32_t light = max44009_read_light(&m_twi);
    mesh_tx_light(light);
    #if(NRF_LOG_LEVEL <= NRF_LOG_LEVEL_INFO)
        float light_f = light;//TODO check log level
        NRF_LOG_INFO("light = "NRF_LOG_FLOAT_MARKER" lux",NRF_LOG_FLOAT(light_f/1000));
    #endif
}

void app_rtc_handler()
{
    static uint32_t cycle_count = 0;
    static const uint32_t period_alive  = 6;
    static const uint32_t period_bme    = 3;
    static const uint32_t period_light  = 3;

    if( (cycle_count % period_alive)==0)
    {
        mesh_tx_alive();
    }
    if( (cycle_count % period_bme)==0)
    {
        read_send_bme();
    }
    if( (cycle_count % period_light)==0)
    {
        read_send_light();
    }


}

int main(void)
{
    uint32_t err_code;

    // ------------------------- Start Init ------------------------- 
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    clocks_start();

    battery_init();

    err_code = mesh_init();
    APP_ERROR_CHECK(err_code);

    twi_init(&m_twi);

    //only allow interrupts to start after init is done
    rtc_config(app_rtc_handler);
    // ------------------------- Start App ------------------------- 

    NRF_LOG_INFO("____________________________");
    NRF_LOG_INFO("Hello from nRF52 Sensors");
    NRF_LOG_INFO("____________________________");

    //twi_scan();

    //max44009_test();

    //err_code = bme280_init(&m_twi);
    //APP_ERROR_CHECK(err_code);
    //NRF_LOG_INFO("bme280_init() done");

    mesh_tx_reset();
    //mesh_wait_tx();

    float v_bat_mili = get_battery();
    NRF_LOG_INFO("V Bat = "NRF_LOG_FLOAT_MARKER" Volts",NRF_LOG_FLOAT(v_bat_mili/1000));

    // ------------------------- Start Events ------------------------- 

    while(true)
    {
        __SEV();
        __WFE();
        __WFE();
    }
}
/*lint -restore */
