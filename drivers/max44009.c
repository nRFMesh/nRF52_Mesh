/** @file max44009.c
 *
 * @author Wassim FILALI
 *
 * @compiler IAR STM8
 *
 *
 * $Date: 29.10.2016 - creation out of refactoring
 * $Revision: 1 
 *
*/

#include "max44009.h"

#include "math.h"

#define NRF_LOG_MODULE_NAME max

#if (MAX_CONFIG_LOG_ENABLED == 1)
#define NRF_LOG_LEVEL MAX_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR MAX_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR MAX_CONFIG_DEBUG_COLOR
#else //MAX_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif //MAX_CONFIG_LOG_ENABLED

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();



static const uint8_t address = 0x4A;
static const nrf_drv_twi_t *p_twi = NULL;

//The MAX44009 does not support auto reg address increment
//reg address have to be sent for every byte either with I²C stop or with a new transaction
uint8_t max44009_read_reg(uint8_t reg)
{
  uint8_t res;
  ret_code_t err_code;
  err_code = nrf_drv_twi_tx(p_twi, address, &reg, 1,true);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_twi_rx(p_twi, address, &res, 1);
  APP_ERROR_CHECK(err_code);
  //NRF_LOG_DEBUG("@ 0x%02x => 0x%02x", reg,res);

	return res;
}

uint16_t max44009_read_luxregs()
{
    //TODO using transfers can ensure no stop between reads for consistent regs content
    return 0;
}

float max44009_data_to_lux(uint8_t high,uint8_t low)
{
    float exponent = (float)(high >> 4);
    float mantissa = (float)(((0x0F&high) << 4)+(0x0F & low));
    NRF_LOG_DEBUG("exponent = "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(exponent));
    NRF_LOG_DEBUG("mantissa = "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(mantissa));
    float highval = pow(2,exponent) * mantissa;
    NRF_LOG_DEBUG("highval = "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(highval));
    float light_lux = highval * 0.045;
    return light_lux;
}

void max44009_test()
{
    NRF_LOG_INFO("-------- test MAX44009 --------");
    uint8_t high,low;
    float res;
    high = 0b00000000;
    low  = 0b0001;
    res = max44009_data_to_lux(high,low);
    NRF_LOG_INFO("0000 0000 0001 => (? 0.045 ) "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(res));

    high = 0b00000001;
    low  = 0b0000;
    res = max44009_data_to_lux(high,low);
    NRF_LOG_INFO("0000 0001 0000 => (? 0.72 ) "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(res));

    high = 0b00010001;
    low  = 0b0001;
    res = max44009_data_to_lux(high,low);
    NRF_LOG_INFO("0001 0001 0001 => (? 1.53 ) "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(res));

    high = 0b11101111;
    low  = 0b1111;
    res = max44009_data_to_lux(high,low);
    NRF_LOG_INFO("1110 1111 1111 => (? 188006 )"NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(res));

    high = 0b11101111;
    low  = 0b1110;
    res = max44009_data_to_lux(high,low);
    NRF_LOG_INFO("1110 1111 1110 => (? 187269 ) "NRF_LOG_FLOAT_MARKER,NRF_LOG_FLOAT(res));
}

uint32_t max44009_read_light(nrf_drv_twi_t *l_twi)
{
    p_twi = l_twi;
    uint8_t high = max44009_read_reg(0x03);
    uint8_t low  = max44009_read_reg(0x04);
    uint8_t high1 = max44009_read_reg(0x03);
    uint8_t low1  = max44009_read_reg(0x04);
    if((high!=high1) || (low!=low1))//error edge encountred, repeat read, won't happen twice in row
    {
        high = max44009_read_reg(0x03);
        low  = max44009_read_reg(0x04);
    }
    uint32_t res = (uint32_t) (max44009_data_to_lux(high,low) * 1000);
	return res;
}

