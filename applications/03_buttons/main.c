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

#include "nrf_pwr_mgmt.h"

//for the log
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//For the GPIO events
#include "nrf_drv_gpiote.h"

// --------------------- inputs from sdk_config --------------------- 
// ---> TWI0_ENABLED ---> TWI1_ENABLED

//drivers - apps
#include "clocks.h"
#include "battery.h"
#include "mesh.h"

static const uint8_t g_btn_list[BUTTONS_NUMBER] = BUTTONS_LIST;

#if (BUTTONS_NUMBER > 8)
    #error Non supported button number;
#endif


void app_mesh_handler(message_t* msg)
{
    NRF_LOG_INFO("app_mesh_handler()");
}

void read_send_battery()
{
    uint16_t v_bat_mili = get_battery();
    mesh_tx_battery(v_bat_mili);
}


void app_rtc_handler()
{
    static uint32_t cycle_count = 0;
    static const uint32_t period_bat    = 60;
    static const uint32_t offset_bat    = 0;
    static const uint32_t period_alive  = 3;
    static const uint32_t offset_alive  = 2;

    if( ((cycle_count+offset_bat) % period_bat)==0)
    {
        read_send_battery();
    }
    if( ((cycle_count+offset_alive) % period_alive)==0)
    {
        mesh_tx_alive();
    }
    mesh_wait_tx();

    cycle_count++;

}

void test_ping()
{
    while(1)
    {
        nrf_delay_ms(100);
        mesh_tx_alive();
        mesh_wait_tx();
    }
}

uint8_t buttons_array_read()
{
    uint8_t res = 0;
    for(int i=0;i<BUTTONS_NUMBER;i++)
    {
        uint8_t pio_val = (uint8_t)nrf_gpio_pin_read(g_btn_list[i]);
        res = res | (pio_val << i );
    }
    return res;
}

void gpio_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint8_t btn_array_val = buttons_array_read();
    mesh_tx_button(btn_array_val);
    mesh_wait_tx();
}

void gpio_init( void )
{
    //  --------  Configure the PIO interrupt accordingly  --------  
    nrf_drv_gpiote_init();
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    config.pull = NRF_GPIO_PIN_PULLUP;
    for(int i=0;i< BUTTONS_NUMBER;i++)
    {
        nrf_drv_gpiote_in_init(g_btn_list[i], &config, gpio_interrupt);
        nrf_drv_gpiote_in_event_enable(g_btn_list[i], true);
    }
}

int main(void)
{
    uint32_t err_code;

    // ------------------------- Start Init ------------------------- 
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);

    clocks_start();

    battery_init();

    err_code = mesh_init(app_mesh_handler,NULL);//in case of rf rx, cmd handler not required
    APP_ERROR_CHECK(err_code);

    //only allow interrupts to start after init is done
    rtc_config(app_rtc_handler);

    gpio_init();

    // ------------------------- Start App ------------------------- 

    NRF_LOG_INFO("____________________________");
    NRF_LOG_INFO("Hello from nRF52 Sensors");
    NRF_LOG_INFO("____________________________");

    //transmission is not re-entrant
    //nrf_delay_ms(100);
    mesh_tx_reset();
    mesh_wait_tx();

    //test_ping();
    //read_send_battery();//could not be sent after rest with and without wait_tx

    // ------------------------- Start Events ------------------------- 

    while(true)
    {
        nrf_pwr_mgmt_run();
    }
}
/*lint -restore */
