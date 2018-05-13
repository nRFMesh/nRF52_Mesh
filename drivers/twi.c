
#include "twi.h"

#include "sdk_common.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "boards.h"


// --------------------- inputs from sdk_config --------------------- 
// ---> TWI_DEFAULT_CONFIG_FREQUENCY


//-------------------------------------------------------------------

/* TWI instance. */
static const nrf_drv_twi_t *p_twi = NULL;


 /* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127


//TODO optimisation
// * 400K did not work on 10 cm test wire lines => using 100K, to check again with PCB
void twi_init(const nrf_drv_twi_t *l_twi)
{
    ret_code_t err_code;

    p_twi = l_twi;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = I2C_SCL,
       .sda                = I2C_SDA,
       .frequency          = TWI_DEFAULT_CONFIG_FREQUENCY,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(p_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(p_twi);
}

//found BME280  @ 0x76
//found MX44009 @ 0x4A
void twi_scan()
{
    ret_code_t err_code;
    uint8_t address;
    uint8_t sample_data;
    bool detected_device = false;

    for (address = 1; address <= TWI_ADDRESSES; address++)
    {
        err_code = nrf_drv_twi_rx(p_twi, address, &sample_data, sizeof(sample_data));
        if (err_code == NRF_SUCCESS)
        {
            detected_device = true;
            NRF_LOG_INFO("TWI device detected at address 0x%x.", address);
        }
        NRF_LOG_FLUSH();
    }

    if (!detected_device)
    {
        NRF_LOG_INFO("No device was found.");
        NRF_LOG_FLUSH();
    }
}
