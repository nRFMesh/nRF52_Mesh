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

// --------------------- inputs from sdk_config --------------------- 
// ---> TWI0_ENABLED ---> TWI1_ENABLED

//drivers - apps
#include "mpu6050.h"
#include "clocks.h"
#include "battery.h"
#include "twi.h"
#include "mesh.h"

bool is_twi_clocks = false;

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

void app_mesh_handler(message_t* msg)
{
    NRF_LOG_INFO("app_mesh_handler()");
}

void read_send_battery()
{
    uint16_t v_bat_mili = get_battery();
    mesh_tx_battery(v_bat_mili);
    #if(NRF_LOG_LEVEL <= NRF_LOG_LEVEL_INFO)
        float v_bat_mili_f  = v_bat_mili;
        NRF_LOG_INFO("V Bat = "NRF_LOG_FLOAT_MARKER" Volts",NRF_LOG_FLOAT(v_bat_mili_f/1000));
    #endif
}

void read_send_accell()
{
    uint8_t accell_data[6];
    mpu_get_accell_data(accell_data);
    mesh_bcast_data(Mesh_Pid_accell,accell_data,6);
}

//always 0
void read_send_gyro()
{
    uint8_t gyro_data[6];
    mpu_get_gyro_data(gyro_data);
    char text[40];
    rx_gyro(text,gyro_data,6);
    mesh_bcast_text(text);
}

void read_send_motion()
{
    char text[20];

    float x,y,z;
    mpu_get_accell(&x,&y,&z);
    x = x*x;
    y = y*y;
    z = z*z;
    if(x > y)
    {
        if(x > z)
        {
            sprintf(text,"Motion X:%0.3f;%0.3f;%0.3f",x,y,z);
        }
    }
    else if(y > z)
    {
        sprintf(text,"Motion Y:%0.3f;%0.3f;%0.3f",x,y,z);
    }
    else
    {
        sprintf(text,"Motion Z:%0.3f;%0.3f;%0.3f",x,y,z);
    }

    mesh_bcast_text(text);
    mesh_wait_tx();
}

void app_mpu_handler(uint8_t event)
{
    bool do_manage_twi_clocks = false;
    if(!is_twi_clocks)
    {
        clocks_restart();
        twi_restart();
        do_manage_twi_clocks = true;
    }

    read_send_accell();
    //read_send_motion();
    nrf_delay_ms(500);
    read_send_accell();
    mesh_wait_tx();
    
    if(do_manage_twi_clocks)
    {
        twi_stop();
        clocks_stop();
        is_twi_clocks = false;
    }
}


void app_rtc_handler()
{
    bool do_manage_twi_clocks = false;
    static uint32_t cycle_count = 0;
    static const uint32_t period_bat    = 60;
    static const uint32_t offset_bat    = 0;
    static const uint32_t period_alive  = 3;
    static const uint32_t offset_alive  = 2;

    if(!is_twi_clocks)
    {
        clocks_restart();
        twi_restart();
        do_manage_twi_clocks = true;
    }

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

    if(do_manage_twi_clocks)
    {
        twi_stop();
        clocks_stop();
        is_twi_clocks = false;
    }

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

    twi_init(&m_twi);
    is_twi_clocks = true;

    //this init needs teh twi to be enabled
    mpu_init(&m_twi);
    mpu_motion_init(app_mpu_handler);
    //mpu_cycle();

    //only allow interrupts to start after init is done
    rtc_config(app_rtc_handler);

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
    //an interrupt might have happened after which the twi clocks sent off
    if(is_twi_clocks)
    {
        twi_stop();
        clocks_stop();//release the hf clock
        is_twi_clocks = false;
    }

    //enable mpu interrupt only after stopping the twi as the interrupt restart it

    while(true)
    {
        nrf_pwr_mgmt_run();
    }
}
/*lint -restore */
