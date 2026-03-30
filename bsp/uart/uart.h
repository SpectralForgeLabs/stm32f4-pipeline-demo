#ifndef __INCuart_h
#define __INCuart_h

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */


/** Global/Static variables */
extern SemaphoreHandle_t uart_tx_sem;
extern QueueHandle_t uart_rx_queue;

extern uint8_t uart_buf[256];

int uart_init(void);
void uart_printf(const char *fmt, ...);
void uart_write(uint8_t * data, uint16_t len);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCuart_h */