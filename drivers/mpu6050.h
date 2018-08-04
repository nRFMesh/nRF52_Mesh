#ifndef MPU6050_APP_H
#define MPU6050_APP_H

#include <stdint.h>

#include "nrf_drv_twi.h"

typedef void (*app_mpu_handler_t)(uint8_t);

void mpu_dump_regs();

void mpu_init(const nrf_drv_twi_t *l_twi);//also wakes up

void mpu_cycle();//cyclically wakeup from interrupt and update reagisters left ready to be read
void mpu_motion_init(app_mpu_handler_t handler);

void mpu_get_accell(float *x,float *y,float *z);
void mpu_get_accell_data(uint8_t *data);
void mpu_get_gyro_data(uint8_t *data);

//experimental
void mpu_wakeup();
void mpu_sleep();

#endif /*MPU6050_APP_H*/