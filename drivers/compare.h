#ifndef __COMPARE_H__
#define __COMPARE_H__


#include <stdint.h>

typedef void (*app_compare_handler_t)();

typedef struct 
{
    uint32_t compare0,  compare1,   compare2;
    uint32_t cycle,     offset1,    offset2;
    app_compare_handler_t call1;
    app_compare_handler_t call2;
}apptimer_config_t;

void compare_init(apptimer_config_t config);

#endif /*__COMPARE_H__*/
