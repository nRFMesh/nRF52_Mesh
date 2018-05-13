#ifndef __CLOCKS_H__
#define __CLOCKS_H__

typedef void (*app_rtc_handler_t)(void);

void clocks_start();

void rtc_config(app_rtc_handler_t handler);

#endif /*__CLOCKS_H__*/