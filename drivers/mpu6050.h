#ifndef MPU6050_APP_H
#define MPU6050_APP_H

#include <stdint.h>

#include "nrf_drv_twi.h"

void mpu_dump_regs();

void mpu_init(const nrf_drv_twi_t *l_twi);//also wakes up

void mpu_wakeup();

void mpu_sleep();

void mpu_get_accell_data(uint8_t *data);

#endif /*MPU6050_APP_H*/