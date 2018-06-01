
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

/**
 * @brief Construct a new nrf serial drv uart config def object
 * RX_PIN_NUMBER comes from the board definition
 * TX_PIN_NUMBER comes from the board definition
 * CTS,RTS  : Forced to NRF_UARTE_PSEL_DISCONNECTED
 * Flow     : Forced to NRF_UARTE_HWFC_DISABLED
 * 
 */
NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uart0_drv_config,
                      RX_PIN_NUMBER, TX_PIN_NUMBER,
                      NRF_UARTE_PSEL_DISCONNECTED, NRF_UARTE_PSEL_DISCONNECTED,
                      NRF_UARTE_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
                      APP_SERIAL_BAUDRATE,
                      UART_DEFAULT_CONFIG_IRQ_PRIORITY);

#define SERIAL_FIFO_TX_SIZE 128
#define SERIAL_FIFO_RX_SIZE 32

NRF_SERIAL_QUEUES_DEF(serial_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);


#define SERIAL_BUFF_TX_SIZE 10
#define SERIAL_BUFF_RX_SIZE 2

NRF_SERIAL_BUFFERS_DEF(serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

//Forward declaration
static void ser_event_handler(nrf_serial_t const * p_serial,nrf_serial_event_t event);

NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_DMA,
                      &serial_queues, &serial_buffs, ser_event_handler, sleep_handler);


NRF_SERIAL_UART_DEF(serial_uart, APP_SERIAL_INSTANCE);

static app_serial_handler_t m_app_serial_handler;
static app_serial_tx_handler_t m_app_serial_tx_handler;

static char uart_cmd[128];
static uint8_t uart_cmd_count=0;

uint32_t ser_evt_tx_count = 0;
uint32_t ser_evt_rx_count = 0;
uint32_t ser_evt_drv_err_count = 0;
uint32_t ser_evt_fifo_err_count = 0;
uint32_t ser_tx_err_count = 0;

void serial_rx_handler(const char* msg,uint8_t size)
{
    for(int i=0;i<size;i++)
    {
        char c = msg[i];
        if( (c == '\r') || (c == '\n') || (c == 0) )
        {
            if(uart_cmd_count > 0)
            {
                m_app_serial_handler(uart_cmd,uart_cmd_count);
                uart_cmd_count = 0;
            }
        }
        else
        {
            uart_cmd[uart_cmd_count++] = c;
        }
    }
}

/**
 * @brief the DMA handles a batch of 4 bytes and triggers an event after every 4 bytes
 * 
 * @param p_serial : given back the serial instance, not required if global used
 * @param event : either of : NRF_SERIAL_EVENT_TX_DONE,..RX_DATA,..DRV_ERR,..FIFO_ERR
 */
static void ser_event_handler(nrf_serial_t const * p_serial,nrf_serial_event_t event)
{
    switch(event)
    {
        case NRF_SERIAL_EVENT_TX_DONE:
            ser_evt_tx_count++;
            m_app_serial_tx_handler();
        break;
        case NRF_SERIAL_EVENT_RX_DATA:
            {
                ser_evt_rx_count++;
                size_t read;
                char buffer[16];//max expected per event
                nrf_serial_read(&serial_uart, &buffer, sizeof(buffer), &read, 0);
                serial_rx_handler(buffer,read);
            }
        break;
        case NRF_SERIAL_EVENT_DRV_ERR:
            ser_evt_drv_err_count++;
        break;
        case NRF_SERIAL_EVENT_FIFO_ERR:
            ser_evt_fifo_err_count++;
        break;
    }
}

void ser_init(app_serial_handler_t handler,app_serial_tx_handler_t tx_handler)
{
    ret_code_t ret;

    m_app_serial_handler = handler;
    m_app_serial_tx_handler = tx_handler;

    ret = nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);
    APP_ERROR_CHECK(ret);

    //ret = nrf_serial_read(&serial_uart, &uart_cmd, 4, NULL, 0);
    //APP_ERROR_CHECK(ret);
}

/**
 * @brief This function is non blocking, using 0 as parameter
 * Internal buffers and fifos are thus beeing used
 * 
 * @param message A null terminated string, this address will directly be used by the DAM
 * pointer, so do not pass any temporary local variable
 */
//Non blocking mode, using 0 as parameter
void ser_send(char* message)
{
    ret_code_t ret;
    do
    {
        //has a mutex protection over the TX queue, can be busy, thus retry
        //15 ms is max for 120 chars @115200
        ret = nrf_serial_write(&serial_uart,message,strlen(message),NULL,0);
    }while(ret == NRF_ERROR_BUSY);
    if(ret == NRF_ERROR_TIMEOUT)
    {
        ser_tx_err_count++;
    }
    else if(ret != NRF_SUCCESS)
    {
        ser_tx_err_count++;
    }
}
#else
    void ser_init(app_serial_handler_t handler)
    {
    }
    void ser_send(char* message)
    {
    }
#endif /*APP_SERIAL_ENABLED*/

