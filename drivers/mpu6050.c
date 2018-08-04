

#include "mpu6050.h"
#include "mpu60x0_register_map.h"

#include "nrf52_sensortag.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"

static const uint8_t expected_who_am_i = 0x68U; // !< Expected value to get from WHO_AM_I register.
static uint8_t       m_device_address;          // !< Device address in bits [7:1]

static const nrf_drv_twi_t *p_twi = NULL;
static app_mpu_handler_t g_app_mpu_handler;



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
    if (mpu6050_register_read(MPU_REG_WHO_AM_I) == expected_who_am_i)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//should be used when a single measure is required, but have to wait the measure
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

void mpu_get_accell_data(uint8_t *data)
{
    //clear any pending interrupt, used when awaken from interrupt after the user sets back the twi clocks
    mpu6050_register_read(MPU_REG_INT_STATUS);//read is enough to clear
    //XH,XL ; YH,YL ; ZH,ZL
    mpu6050_read_burst(MPU_REG_ACCEL_XOUT_H,6,data);
}

void mpu_get_accell(float *x,float *y,float *z)
{
    uint8_t data[6];
    //XH,XL ; YH,YL ; ZH,ZL
    mpu6050_read_burst(MPU_REG_ACCEL_XOUT_H,6,data);
    int16_t 	accell_x =  data[0] << 8;
                accell_x |= data[1];
    int16_t 	accell_y =  data[2] << 8;
                accell_y |= data[3];
    int16_t 	accell_z =  data[4] << 8;
                accell_z |= data[5];
    *x =  accell_x;
    *x /= 16384;
    *y =  accell_y;
    *y /= 16384;
    *z =  accell_z;
    *z /= 16384;
}

void mpu_get_gyro_data(uint8_t *data)
{
    //XH,XL ; YH,YL ; ZH,ZL
    mpu6050_read_burst(MPU_REG_GYRO_XOUT_H,6,data);
}

void mpu6050_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    g_app_mpu_handler(0);
}

bool mpu6050_init(uint8_t device_address)
{
    bool transfer_succeeded = true;
    m_device_address = device_address;

    // Read and verify product ID
    transfer_succeeded &= mpu6050_verify_product_id();

    mpu6050_register_write(MPU_REG_PWR_MGMT_1,0x80);//DEVICE_RESET
    nrf_delay_ms(100);
    // Do a reset on signal paths
    uint8_t reset_value = 0x04U | 0x02U | 0x01U; // Resets gyro, accelerometer and temperature sensor signal paths.
    transfer_succeeded &= mpu6050_register_write(MPU_REG_SIGNAL_PATH_RESET, reset_value);
    nrf_delay_ms(100);

    return transfer_succeeded;
}

void mpu_cycle()
{
    // 1KHz / (1,256) => 1KHz,3.9 Hz
    //TODO to test is it has an impact on power or sampling rate ?
    //mpu6050_register_write(MPU_REG_SMPLRT_DIV,200);//=> matches the 5 Hz

    //config = 6 ; Bandwidth : Accel 5 Hz delay 19 ms, Gyro 5 Hz delay 18.6 ms, FS 1 KHz
    //EXT_SYNC_SET input disabled for FSYNC pin
    mpu6050_register_write(MPU_REG_CONFIG,6);
    //FS_SEL = 0 => 250°/s (lowest)
    mpu6050_register_write(MPU_REG_GYRO_CONFIG,0); // same as reset value
    //AFS_SEL = 0 => +- 2g (lowest)
    mpu6050_register_write(MPU_REG_ACCEL_CONFIG,0); // same as reset value

    // DEVICE_RESET,SLEEP,CYCLE, - ; ; TEMP_DIS, CLKSEL[2:0]
    //wakeup
    //0x08 : TEMP_DIS => disable temperature
    //0x20 : CYCLE wake up every period
    //0x00 : Internal 8 MHz oscillator
    mpu6050_register_write(MPU_REG_PWR_MGMT_1,0x28);//8 : TEMP_DIS - awake
    //channels standby modes 0x38 Accel  ; 0x07 Gyro
    //LP_WAKE_CTRL 0 = 1.25 Hz
    mpu6050_register_write(MPU_REG_PWR_MGMT_2,0x07);//0x07 Gyro in standby

    //disable interrupts
    mpu6050_register_write(MPU_REG_INT_PIN_CFG, 0);
    mpu6050_register_write(MPU_REG_INT_ENABLE, 0);
}

void mpu_motion_init(app_mpu_handler_t handler)
{
    g_app_mpu_handler = handler;
    // DEVICE_RESET,SLEEP,CYCLE, - ; ; TEMP_DIS, CLKSEL[2:0]
    mpu6050_register_write(MPU_REG_PWR_MGMT_1,0x08);
    
    //LP_WAKE_CTRL[1:0] STBY_XA,STBY_YA ; ; STBY_ZA,STBY_XG ,STBY_YG ,STBY_ZG
    mpu6050_register_write(MPU_REG_PWR_MGMT_2,0x00);


    //EXT_SYNC_SET[2:0],DLP_CFG[2:0]
    mpu6050_register_write(MPU_REG_CONFIG,0);
    //FS_SEL = 0 => 250°/s (lowest)
    mpu6050_register_write(MPU_REG_GYRO_CONFIG,0);
    //AFS_SEL = 0 => +- 2g (lowest)
    mpu6050_register_write(MPU_REG_ACCEL_CONFIG,0);

    //  --------  Configure the MPU for correct interrupt  --------  
    //INT_LEVEL=0 (activelow), INT_OPEN=0 (pushpull), INT_LATCH_EN=0 (50us pulse), INT_RD_CLEAR=0 (clear on read status)
    //NT_RD_CLEAR=0 (FSYNC do not care), FSYNC_INT_LEVEL=0 (do not care), FSYNC_INT_EN=0 (do not care), I2C_BYPASS_EN=0 (do not care)
    //actually the interrupt INT pin of the MPU_6050 is always enabled
    mpu6050_register_write(MPU_REG_INT_PIN_CFG, 0);
    
    mpu6050_register_write(MPU_REG_INT_ENABLE, 0x40);           //0x40 Magic number defined by datasheet Spec page 33 8.1 Motion Interrupt
    
    mpu6050_register_write(MPU_REG_MOT_DUR, 0x01);              //defined by datasheet Spec page 33 8.1 Motion Interrupt

    uint8_t threshold = 2;//1-255
    mpu6050_register_write(MPU_REG_MOT_THR, threshold);       //defined by datasheet Spec page 33 8.1 Motion Interrupt

    //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set 
    //both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. ) 
    //mpu6050_register_write(MPU_REG_MOT_DETECT_CTRL, 0x15);//TODO define more details about this undocumented register

    nrf_delay_ms(1);                                            //defined by datasheet Spec page 33 8.1 Motion Interrupt

    //Note : 7 is reserved and 0x1A is CONFIG not ACCEL_CONFIG,
    mpu6050_register_write(MPU_REG_CONFIG,7);                   //defined by datasheet Spec page 33 8.1 Motion Interrupt

    //LP_WAKE_CTRL[1:0] STBY_XA,STBY_YA ; ; STBY_ZA,STBY_XG ,STBY_YG ,STBY_ZG
    mpu6050_register_write(MPU_REG_PWR_MGMT_2,0x07);//0 lowest freq 1.25 Hz, Gyro standby
    // DEVICE_RESET,SLEEP,CYCLE, - ; ; TEMP_DIS, CLKSEL[2:0]
    mpu6050_register_write(MPU_REG_PWR_MGMT_1,0x28);

    //  --------  Configure the PIO interrupt accordingly  --------  
    nrf_drv_gpiote_init();
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    config.pull = NRF_GPIO_PIN_NOPULL;
    nrf_drv_gpiote_in_init(SENSOR_INT, &config, mpu6050_interrupt);
    nrf_drv_gpiote_in_event_enable(SENSOR_INT, true);//true = enable

    //clear any pending interrupt to enable newer ones
    mpu6050_register_read(MPU_REG_INT_STATUS);//read is enough to clear
}

void mpu_init(const nrf_drv_twi_t *l_twi)
{
    p_twi = l_twi;

    mpu6050_init(0x68);//verify id and reset
}

