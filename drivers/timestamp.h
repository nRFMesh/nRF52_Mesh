#ifndef __TIMESTAMP_TIMER_H__
#define __TIMESTAMP_TIMER_H__


#include <stdint.h>

void timestamp_init();

//timestamp_get() not efficient as triggers a capture event then ready the capture reg, no direct access to counter
//actually the good thing is that it foreces the user to use TASKs to trigger teh capture and not do it by soft
uint32_t timestamp_get();
void timestamp_reset();

#endif /*__TIMESTAMP_TIMER_H__*/
