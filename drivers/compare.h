#ifndef __COMPARE_H__
#define __COMPARE_H__


#include <stdint.h>

typedef void (*app_compare_handler_t)();

typedef struct 
{
    uint32_t cycle,     offset0,    offset1;
    app_compare_handler_t call0, call1;
}apptimer_config_t;

void compare_init(apptimer_config_t config);

#endif /*__COMPARE_H__*/
