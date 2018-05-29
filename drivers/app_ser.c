
#include "app_ser.h"

#include "sdk_common.h"

#include "boards.h"

#include "nrf_serial.h"

#include "app_error.h"
#include "app_util.h"

#if(APP_SERIAL_ENABLED == 1)

static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}


NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uart0_drv_config,
                      APP_SERIAL_RX_PIN, APP_SERIAL_TX_PIN,
                      RTS_PIN_NUMBER, CTS_PIN_NUMBER,
                      HWFC, NRF_UART_PARITY_EXCLUDED,
                      APP_SERIAL_BAUDRATE,
                      UART_DEFAULT_CONFIG_IRQ_PRIORITY);

#define SERIAL_FIFO_TX_SIZE 128
#define SERIAL_FIFO_RX_SIZE 32

NRF_SERIAL_QUEUES_DEF(serial_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);


#define SERIAL_BUFF_TX_SIZE 5
#define SERIAL_BUFF_RX_SIZE 2

NRF_SERIAL_BUFFERS_DEF(serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_IRQ,
                      &serial_queues, &serial_buffs, NULL, sleep_handler);


NRF_SERIAL_UART_DEF(serial_uart, APP_SERIAL_INSTANCE);

void ser_init()
{
    ret_code_t ret;

    ret = nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);
    APP_ERROR_CHECK(ret);
}

//Non blocking mode, using 0 as parameter
void ser_send(char* message)
{
    nrf_serial_write(&serial_uart,message,strlen(message),NULL,0);
}

#else
void ser_init()
{
}
void ser_send(char* message)
{
}
#endif /*APP_SERIAL_ENABLED*/