/*
 * BOSH BME280 driver.
 *
 * Copyright (c) 2016, Offcode Ltd. All rights reserved.
 * Author: Janne Rosberg <janne@offcode.fi>
 *
 * Reference: BST-BME280-DS001-11 | Revision 1.2 | October 2015
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice,   this list of conditions and the following disclaimer.
 *    * Redistributions in  binary form must  reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the RuuviTag nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND  ANY  EXPRESS  OR  IMPLIED WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO,
 * THE  IMPLIED  WARRANTIES  OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY, OR
 * CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED  TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE, DATA, OR PROFITS;  OR BUSINESS
 * INTERRUPTION)  HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  Changelog
 *  2016-11-17 Otso Jousimaa (otso@ruuvi.com): Port function calls to use Ruuvi SPI driver 
 *  2016-11-18 Otso Jousimaa (otso@ruuvi.com): Add timer to poll data 
 *  2017-01-28 Otso Jousimaa Add comments.
 *  2017-04-06: Add t_sb register value. 
 *  2017-08-12 Otso Jousimaa (otso@ruuvi.com): Add Error checking, IIR filtering
 * 
 *  2018-05-05 wass fila : 
 *        - switched to nRF SDK 15.0.0
 *        - switch interface from SPI to I²C => using an nrf_drv_twi_t
 *        - simplified to low power single shot usecase (no interval, no oversampling)
 *        - safer compensation of the measures after the trigger, read vals in any order multiples times
 */

#include "bme280.h"

#define NRF_LOG_MODULE_NAME bme

#if (BME_CONFIG_LOG_ENABLED == 1)
#define NRF_LOG_LEVEL BME_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR BME_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR BME_CONFIG_DEBUG_COLOR
#else //BME_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif //BME_CONFIG_LOG_ENABLED

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

struct bme280_driver bme280; /* global instance */

static const uint8_t address = 0x76;

static const nrf_drv_twi_t *p_twi = NULL;

/** state variable **/
static uint8_t current_mode = BME280_MODE_SLEEP;


uint8_t bme280_read_reg(uint8_t reg)
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

BME280_Ret bme280_read_burst(uint8_t start, uint8_t length, uint8_t* buffer)
{
  ret_code_t err_code;
  err_code = nrf_drv_twi_tx(p_twi, address, &start, 1,true);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_twi_rx(p_twi, address, buffer, length);
  APP_ERROR_CHECK(err_code);

  return 0;
}

BME280_Ret bme280_write_reg(uint8_t reg, uint8_t value)
{
  ret_code_t err_code;
  uint8_t data[2];
  data[0] = reg;
  data[1] = value;
  err_code = nrf_drv_twi_tx(p_twi, address, data, 2,false);
  APP_ERROR_CHECK(err_code);
  return 0;
}

BME280_Ret bme280_init(const nrf_drv_twi_t *l_twi)
{
  NRF_LOG_INFO("bme280_init()");
  //Return error if not in sleep
  if(BME280_MODE_SLEEP != current_mode){ return BME280_RET_ILLEGAL; }

  p_twi = l_twi;

  uint8_t reg = bme280_read_reg(BME280REG_ID);
  bme280.sensor_available = false;

	if (BME280_ID_VALUE == reg)
  {
	  bme280.sensor_available = true;
    NRF_LOG_INFO("bme280 available");
  }
	else
  {
    NRF_LOG_WARNING("bme280 not found");
    //Assume that 0x00 means no response. Other values are self-test errors (invalid who-am-i).
		return (0x00 == reg) ? BME280_RET_ERROR : BME280_RET_ERROR_SELFTEST;
  }

  // load calibration data...
  bme280.cp.dig_T1  = bme280_read_reg(BME280REG_CALIB_00);          //0x82
  bme280.cp.dig_T1 |= bme280_read_reg(BME280REG_CALIB_00+1) << 8;
  bme280.cp.dig_T2  = bme280_read_reg(BME280REG_CALIB_00+2);
  bme280.cp.dig_T2 |= bme280_read_reg(BME280REG_CALIB_00+3) << 8;
  bme280.cp.dig_T3  = bme280_read_reg(BME280REG_CALIB_00+4);
  bme280.cp.dig_T3 |= bme280_read_reg(BME280REG_CALIB_00+5) << 8;

  bme280.cp.dig_P1  = bme280_read_reg(BME280REG_CALIB_00+6);
  bme280.cp.dig_P1 |= bme280_read_reg(BME280REG_CALIB_00+7) << 8;
  bme280.cp.dig_P2  = bme280_read_reg(BME280REG_CALIB_00+8);
  bme280.cp.dig_P2 |= bme280_read_reg(BME280REG_CALIB_00+9) << 8;

  bme280.cp.dig_P3  = bme280_read_reg(BME280REG_CALIB_00+10);       //0x92
  bme280.cp.dig_P3 |= bme280_read_reg(BME280REG_CALIB_00+11) << 8;
  bme280.cp.dig_P4  = bme280_read_reg(BME280REG_CALIB_00+12);
  bme280.cp.dig_P4 |= bme280_read_reg(BME280REG_CALIB_00+13) << 8;
  bme280.cp.dig_P5  = bme280_read_reg(BME280REG_CALIB_00+14);
  bme280.cp.dig_P5 |= bme280_read_reg(BME280REG_CALIB_00+15) << 8;
  bme280.cp.dig_P6  = bme280_read_reg(BME280REG_CALIB_00+16);
  bme280.cp.dig_P6 |= bme280_read_reg(BME280REG_CALIB_00+17) << 8;
  bme280.cp.dig_P7  = bme280_read_reg(BME280REG_CALIB_00+18);
  bme280.cp.dig_P7 |= bme280_read_reg(BME280REG_CALIB_00+19) << 8;

  bme280.cp.dig_P8  = bme280_read_reg(BME280REG_CALIB_00+20);       //0x9C
  bme280.cp.dig_P8 |= bme280_read_reg(BME280REG_CALIB_00+21) << 8;
  bme280.cp.dig_P9  = bme280_read_reg(BME280REG_CALIB_00+22);
  bme280.cp.dig_P9 |= bme280_read_reg(BME280REG_CALIB_00+23) << 8;

  bme280.cp.dig_H1  = bme280_read_reg(0xA1);
  bme280.cp.dig_H2  = bme280_read_reg(0xE1);
  bme280.cp.dig_H2 |= bme280_read_reg(0xE2) << 8;
  bme280.cp.dig_H3  = bme280_read_reg(0xE3);

  bme280.cp.dig_H4  = bme280_read_reg(0xE4) << 4;		// 11:4
  bme280.cp.dig_H4 |= bme280_read_reg(0xE5) & 0x0f;	// 3:0

  bme280.cp.dig_H5  = bme280_read_reg(0xE5) >> 4;		// 3:0
  bme280.cp.dig_H5 |= bme280_read_reg(0xE6) << 4;		// 11:4

  bme280.cp.dig_H6  = bme280_read_reg(0xE7);
 
  return BME280_RET_OK;
}


int bme280_is_measuring(void)
{
  uint8_t s;

  s = bme280_read_reg(BME280REG_STATUS);
  if (s & 0b00001000)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/**
 * @brief Read new raw values.
 */
BME280_Ret bme280_read_measurements()
{

  if(!bme280.sensor_available) { return BME280_RET_ERROR;  }
  uint8_t data[BME280_BURST_READ_LENGTH];
  
  BME280_Ret err_code = bme280_read_burst(BME280REG_PRESS_MSB, BME280_BURST_READ_LENGTH, data);

  bme280.adc_h = data[7] + ((uint32_t)data[6] << 8);

  bme280.adc_t  = (uint32_t) data[5] >> 4;
  bme280.adc_t |= (uint32_t) data[4] << 4;
  bme280.adc_t |= (uint32_t) data[3] << 12;

  bme280.adc_p  = (uint32_t) data[2] >> 4;
  bme280.adc_p |= (uint32_t) data[1] << 4;
  bme280.adc_p |= (uint32_t) data[0] << 12;

  return err_code;
}


static uint32_t compensate_P_int64(int32_t adc_P)
{
	int64_t var1, var2, p;

	var1 = ((int64_t)bme280.t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)bme280.cp.dig_P6;
	var2 = var2 + ((var1*(int64_t)bme280.cp.dig_P5) << 17);
	var2 = var2 + (((int64_t)bme280.cp.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)bme280.cp.dig_P3) >> 8) + ((var1 * (int64_t)bme280.cp.dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bme280.cp.dig_P1) >> 33;
	if (var1 == 0) {
		return 0;
	}

	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)bme280.cp.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)bme280.cp.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)bme280.cp.dig_P7) << 4);

	return (uint32_t)p;
}
static uint32_t compensate_H_int32(int32_t adc_H)
{
	int32_t v_x1_u32r;

	v_x1_u32r = (bme280.t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)bme280.cp.dig_H4) << 20) - (((int32_t)bme280.cp.dig_H5) * v_x1_u32r)) +
		       ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)bme280.cp.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)bme280.cp.dig_H3)) >> 11) +
		       ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)bme280.cp.dig_H2) + 8192) >> 14));

	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme280.cp.dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

	return (uint32_t)(v_x1_u32r >> 12);
}
static int32_t compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;

	var1 = ((((adc_T>>3) - ((int32_t)bme280.cp.dig_T1<<1))) * 
               ((int32_t)bme280.cp.dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)bme280.cp.dig_T1)) *
               ((adc_T>>4) - ((int32_t)bme280.cp.dig_T1))) >> 12) * 
               ((int32_t)bme280.cp.dig_T3)) >> 14;

	bme280.t_fine = var1 + var2;

	T = (bme280.t_fine * 5 + 128) >> 8;
	
  return T;
}

// For optimisation purpose, oversampling is set once on measure triggering
//any change should be set in local variables not latent in sensor's registers
//that avoid reads before writes
void bme280_measure()
{
  //trigger the start of measurments in the sensor
  bme280_write_reg(BME280REG_CTRL_HUM, 0x01);//oversampling hum x1
  bme280_write_reg(BME280REG_CTRL_MEAS, 0x20 | 0x04 | 0x01);//t_x1 | p_x1 | forced

  uint8_t count = 0;
  const uint8_t max_wait = 30;//x1 temp,hum,press is ~ 18 cycles
  int is_measuring;
  do
  {
    is_measuring = bme280_is_measuring();
    count++;
  }while( (is_measuring == 1) && (count < max_wait) );
  if(count == max_wait)
  {
    NRF_LOG_ERROR("BME waiting for measures oveflow");
  }
  else
  {
    NRF_LOG_DEBUG("BME done measuring in %u cycles",count);
    //read the data buffer from the sensor
    bme280_read_measurements();
    bme280.temperature = compensate_T_int32(bme280.adc_t);//temperature must be compensated first
	  bme280.pressure = compensate_P_int64(bme280.adc_p);
  	bme280.humidity = compensate_H_int32(bme280.adc_h);
  }
}

void bme280_dump()
{
  uint8_t regs[4];
  bme280_read_burst(0xF2,4,regs);
  NRF_LOG_INFO("regs 0xF2,3,4,5 : 0x%02x 0x%02x 0x%02x 0x%02x",regs[0],regs[1],regs[2],regs[3]);
}

/**
 * Returns temperature in DegC, resolution is 0.01 DegC.
 * Output value of “2134” equals 21.34 DegC.
 */
int32_t bme280_get_temperature(void)
{
	return bme280.temperature;
}

/**
 * Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format
 * (24 integer bits and 8 fractional bits).
 * Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
 */
uint32_t bme280_get_pressure(void)
{
	return bme280.pressure;
}

/**
 * Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format
 * (22 integer and 10 fractional bits).
 * Output value of “50532” represents 50532/1024 = 49.356 %RH
 */
uint32_t bme280_get_humidity(void)
{
	return bme280.humidity;
}
