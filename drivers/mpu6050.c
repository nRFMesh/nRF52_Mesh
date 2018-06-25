

#include "mpu6050.h"
#include "mpu60x0_register_map.h"


#define ADDRESS_WHO_AM_I          (0x75U) // !< WHO_AM_I register identifies the device. Expected value is 0x68.
#define ADDRESS_SIGNAL_PATH_RESET (0x68U) // !<

static const uint8_t expected_who_am_i = 0x68U; // !< Expected value to get from WHO_AM_I register.
static uint8_t       m_device_address;          // !< Device address in bits [7:1]

static const nrf_drv_twi_t *p_twi = NULL;

uint8_t mpu6050_register_write(uint8_t register_address, uint8_t value)
{
    ret_code_t err_code;
    uint8_t data[2];
    data[0] = register_address;
    data[1] = value;
    err_code = nrf_drv_twi_tx(p_twi, m_device_address, data, 2,false);
    APP_ERROR_CHECK(err_code);
    return 0;
}

uint8_t mpu6050_register_read(uint8_t register_address)
{
    uint8_t res;
    ret_code_t err_code;
    err_code = nrf_drv_twi_tx(p_twi, m_device_address, &register_address, 1,true);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(p_twi, m_device_address, &res, 1);
    APP_ERROR_CHECK(err_code);
    //NRF_LOG_DEBUG("@ 0x%02x => 0x%02x", reg,res);

    return res;
}

uint8_t mpu6050_read_burst(uint8_t start, uint8_t length, uint8_t* buffer)
{
  ret_code_t err_code;
  err_code = nrf_drv_twi_tx(p_twi, m_device_address, &start, 1,true);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_twi_rx(p_twi, m_device_address, buffer, length);
  APP_ERROR_CHECK(err_code);

  return 0;
}

bool mpu6050_verify_product_id(void)
{
    if (mpu6050_register_read(ADDRESS_WHO_AM_I) == expected_who_am_i)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool mpu6050_init(uint8_t device_address)
{
    bool transfer_succeeded = true;

    m_device_address = device_address;

    // Do a reset on signal paths
    uint8_t reset_value = 0x04U | 0x02U | 0x01U; // Resets gyro, accelerometer and temperature sensor signal paths.
    transfer_succeeded &= mpu6050_register_write(ADDRESS_SIGNAL_PATH_RESET, reset_value);

    // Read and verify product ID
    transfer_succeeded &= mpu6050_verify_product_id();

    return transfer_succeeded;
}

void mpu_init(const nrf_drv_twi_t *l_twi)
{
    p_twi = l_twi;

    mpu6050_init(0x68);

    //config = 6 ; Bandwidth : Accel 5 Hz, Gyro 5 Hz
    mpu6050_register_write(MPU_REG_CONFIG,6);
    //FS_SEL = 0 => 250°/s (lowest)
    mpu6050_register_write(MPU_REG_GYRO_CONFIG,0); // same as reset value
    //AFS_SEL = 0 => +- 2g (lowest)
    mpu6050_register_write(MPU_REG_ACCEL_CONFIG,0); // same as reset value
    
    //wakeup
    //0x08 : TEMP_DIS => disable temperature
    //0x20 : CYCLE wake up every period
    //0x00 : Internal 8 MHz oscillator
    mpu6050_register_write(MPU_REG_PWR_MGMT_1,0x28);//8 : TEMP_DIS - awake
    //channels standby modes 0x38 Accel  ; 0x07 Gyro
    //LP_WAKE_CTRL 0 = 1.25 Hz
    mpu6050_register_write(MPU_REG_PWR_MGMT_2,0x07);//0x07 Gyro in standby

}

void mpu_wakeup()
{
    //wakeup
    //8 : TEMP_DIS => disable temperature
    mpu6050_register_write(MPU_REG_PWR_MGMT_1,0x08);//8 : TEMP_DIS - awake
}

void mpu_sleep()
{
    mpu6050_register_write(MPU_REG_PWR_MGMT_1,0x48);//0x48 => sleep
}

void mpu_get_accell_xyz(int8_t *x,int8_t *y,int8_t *z)
{
    uint8_t accell[5];
    //XH,XL ; YH,YL ; ZH,ZL
    mpu6050_read_burst(MPU_REG_ACCEL_XOUT_H,5,accell);//need only high
    //DEBUG_PRINTF("0x%02X 0x%02X ; 0x%02X 0x%02X ; 0x%02X 0x%02X\r\n",
    //                accell[0],accell[1],accell[2],
    //                accell[3],accell[4],accell[5]);
    *x = accell[0];
    *y = accell[2];
    *z = accell[4];
}

void mpu_get_accell_data(uint8_t *data)
{
    //XH,XL ; YH,YL ; ZH,ZL
    mpu6050_read_burst(MPU_REG_ACCEL_XOUT_H,6,data);
}