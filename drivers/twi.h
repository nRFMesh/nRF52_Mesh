#ifndef __TWI_H__
#define __TWI_H__


#include "nrf_drv_twi.h"

void twi_scan();
void twi_init(const nrf_drv_twi_t *p_twi);
void twi_stop();
void twi_restart();

#endif /*__TWI_H__*/
